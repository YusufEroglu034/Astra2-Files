#ifndef _WIN32
#include <sys/time.h>
#endif

#include <cstdlib>
#include <cstring>

#include "AsyncSQL.h"

#ifndef _WIN32
#define MUTEX_LOCK(mtx) pthread_mutex_lock(mtx)
#define MUTEX_UNLOCK(mtx) pthread_mutex_unlock(mtx)
#else
#define MUTEX_LOCK(mtx) ::EnterCriticalSection(mtx)
#define MUTEX_UNLOCK(mtx) ::LeaveCriticalSection(mtx)
#endif

CAsyncSQL::CAsyncSQL()
	: m_stHost(""), m_stUser(""), m_stPassword(""), m_stDB(""), m_stLocale(""),
	m_iMsgCount(0), m_iPort(0), m_bEnd(false),
#ifndef _WIN32
	m_hThread(0), 
#else
	m_hThread(INVALID_HANDLE_VALUE),
#endif
	m_mtxQuery(nullptr), m_mtxResult(nullptr),
	m_iQueryFinished(0), m_ulThreadID(0), m_bConnected(false), m_iCopiedQuery(0)
{
	memset( &m_hDB, 0, sizeof(m_hDB) );

	m_aiPipe[0] = 0;
	m_aiPipe[1] = 0;
}

CAsyncSQL::~CAsyncSQL()
{
	Quit();
	Destroy();
}

void CAsyncSQL::Destroy()
{
	if (m_hDB.host)
	{
		sys_log(0, "AsyncSQL: closing mysql connection.");
		mysql_close(&m_hDB);
		m_hDB.host = nullptr;
	}

	if (m_mtxQuery)
	{
#ifndef _WIN32
		pthread_mutex_destroy(m_mtxQuery);
#else
		::DeleteCriticalSection(m_mtxQuery);
#endif
		delete m_mtxQuery;
		m_mtxQuery = nullptr;
	}

	if (m_mtxResult)
	{
#ifndef _WIN32
		pthread_mutex_destroy(m_mtxResult);
#else
		::DeleteCriticalSection(m_mtxResult);
#endif
		delete m_mtxResult;
		m_mtxQuery = nullptr;
	}
}

#ifndef _WIN32
void * AsyncSQLThread(void * arg)
#else
uint32_t __stdcall AsyncSQLThread(void* arg)
#endif
{
	CAsyncSQL * pSQL = ((CAsyncSQL *) arg);

	if (!pSQL->Connect())
		return nullptr;

	pSQL->ChildLoop();
	return nullptr;
}

bool CAsyncSQL::QueryLocaleSet()
{
	if (0 == m_stLocale.length())
	{
		sys_err("m_stLocale == 0");
		return true;
	}

	else if (m_stLocale == "ascii")
	{
		sys_err("m_stLocale == ascii");
		return true;
	}

	if (mysql_set_character_set(&m_hDB, m_stLocale.c_str()))
	{
		sys_err("cannot set locale %s by 'mysql_set_character_set', errno %u %s", m_stLocale.c_str(), mysql_errno(&m_hDB) , mysql_error(&m_hDB));
		return false; 
	}

	sys_log(0, "\t--mysql_set_character_set(%s)", m_stLocale.c_str());

	return true;
}

bool CAsyncSQL::Connect()
{
	if (0 == mysql_init(&m_hDB))
	{
		fprintf(stderr, "mysql_init failed\n");
		return false;
	}

	if (!m_stLocale.empty())
	{
		if (mysql_options(&m_hDB, MYSQL_SET_CHARSET_NAME, m_stLocale.c_str()) != 0)
		{
			fprintf(stderr, "mysql_option failed : MYSQL_SET_CHARSET_NAME %s ", mysql_error(&m_hDB));
		}
	}

	if (!mysql_real_connect(&m_hDB, m_stHost.c_str(), m_stUser.c_str(), m_stPassword.c_str(), m_stDB.c_str(), m_iPort, nullptr, CLIENT_MULTI_STATEMENTS))
	{
		fprintf(stderr, "mysql_real_connect: %s\n", mysql_error(&m_hDB));
		return false;
	}

	my_bool reconnect = true;

	if (0 != mysql_options(&m_hDB, MYSQL_OPT_RECONNECT, &reconnect))
		fprintf(stderr, "mysql_option: %s\n", mysql_error(&m_hDB));

	fprintf(stdout, "AsyncSQL: connected to %s (reconnect %d)\n", m_stHost.c_str(), m_hDB.reconnect);

	m_ulThreadID = mysql_thread_id(&m_hDB);

	m_bConnected = true;
	return true;
}

bool CAsyncSQL::Setup(CAsyncSQL * sql, bool bNoThread)
{
	return Setup(sql->m_stHost.c_str(),
			sql->m_stUser.c_str(), 
			sql->m_stPassword.c_str(), 
			sql->m_stDB.c_str(), 
			sql->m_stLocale.c_str(), 
			bNoThread,
			sql->m_iPort);
}

bool CAsyncSQL::Setup(const char * c_pszHost, const char * c_pszUser, const char * c_pszPassword, const char * c_pszDB, const char * c_pszLocale, bool bNoThread, int32_t iPort)
{
	m_stHost = c_pszHost;
	m_stUser = c_pszUser;
	m_stPassword = c_pszPassword;
	m_stDB = c_pszDB;
	m_iPort = iPort;

	if (c_pszLocale)
	{
		m_stLocale = c_pszLocale;
		sys_log(0, "AsyncSQL: locale %s", m_stLocale.c_str());
	}

	if (!bNoThread)
	{
#ifndef _WIN32
		m_mtxQuery = new pthread_mutex_t;
		m_mtxResult = new pthread_mutex_t;

		if (0 != pthread_mutex_init(m_mtxQuery, nullptr))
		{
			perror("pthread_mutex_init");
			exit(0);
		}

		if (0 != pthread_mutex_init(m_mtxResult, nullptr))
		{
			perror("pthread_mutex_init");
			exit(0);
		}

		pthread_create(&m_hThread, nullptr, AsyncSQLThread, this);
#else
		m_mtxQuery = new CRITICAL_SECTION;
		m_mtxResult = new CRITICAL_SECTION;

		::InitializeCriticalSection(m_mtxQuery);
		::InitializeCriticalSection(m_mtxResult);

		m_hThread = (HANDLE)::_beginthreadex(nullptr, 0, AsyncSQLThread, this, 0, nullptr);
		if (m_hThread == INVALID_HANDLE_VALUE) {
			perror("CAsyncSQL::Setup");
			return false;
		}
#endif

		return true;
	}
	else
		return Connect();
}

void CAsyncSQL::Quit()
{
	m_bEnd = true;
	m_sem.Release();

#ifndef _WIN32
	if (m_hThread)
	{
		pthread_join(m_hThread, nullptr);
		m_hThread = nullptr;
	}
#else
	if (m_hThread != INVALID_HANDLE_VALUE) {
		::WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = INVALID_HANDLE_VALUE;
	}
#endif
}

SQLMsg * CAsyncSQL::DirectQuery(const char * c_pszQuery)
{
	if (m_ulThreadID != mysql_thread_id(&m_hDB))
	{
		sys_log(0, "MySQL connection was reconnected. querying locale set"); // @correction011
		while (!QueryLocaleSet());
		m_ulThreadID = mysql_thread_id(&m_hDB);
	}

	SQLMsg * p = new SQLMsg;

	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;

	if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
	{
		char buf[1024];

		snprintf(buf, sizeof(buf),
				"AsyncSQL::DirectQuery : mysql_query error: %s\nquery: %s",
				mysql_error(&m_hDB), p->stQuery.c_str());

		sys_err(buf);
		p->uiSQLErrno = mysql_errno(&m_hDB);
	}

	p->Store();
	return p;
}

void CAsyncSQL::AsyncQuery(const char * c_pszQuery)
{
	SQLMsg * p = new SQLMsg;

	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;

	PushQuery(p);
}

void CAsyncSQL::ReturnQuery(const char * c_pszQuery, void * pvUserData)
{
	SQLMsg * p = new SQLMsg;

	p->m_pkSQL = &m_hDB;
	p->iID = ++m_iMsgCount;
	p->stQuery = c_pszQuery;
	p->bReturn = true;
	p->pvUserData = pvUserData;

	PushQuery(p);
}

void CAsyncSQL::PushResult(SQLMsg * p)
{
	MUTEX_LOCK(m_mtxResult);

	m_queue_result.push(p);

	MUTEX_UNLOCK(m_mtxResult);
}

bool CAsyncSQL::PopResult(SQLMsg ** pp)
{
	MUTEX_LOCK(m_mtxResult);

	if (m_queue_result.empty())
	{
		MUTEX_UNLOCK(m_mtxResult);
		return false;
	}

	*pp = m_queue_result.front();
	m_queue_result.pop();
	MUTEX_UNLOCK(m_mtxResult);
	return true;
}

void CAsyncSQL::PushQuery(SQLMsg * p)
{
	MUTEX_LOCK(m_mtxQuery);

	m_queue_query.push(p);

	m_sem.Release();

	MUTEX_UNLOCK(m_mtxQuery);
}

bool CAsyncSQL::PeekQuery(SQLMsg ** pp)
{
	MUTEX_LOCK(m_mtxQuery);

	if (m_queue_query.empty())
	{
		MUTEX_UNLOCK(m_mtxQuery);
		return false;
	}

	*pp = m_queue_query.front();
	MUTEX_UNLOCK(m_mtxQuery);
	return true;
}

bool CAsyncSQL::PopQuery(int32_t iID)
{
	MUTEX_LOCK(m_mtxQuery);

	if (m_queue_query.empty())
	{
		MUTEX_UNLOCK(m_mtxQuery);
		return false;
	}

	m_queue_query.pop();

	MUTEX_UNLOCK(m_mtxQuery);
	return true;
}

bool CAsyncSQL::PeekQueryFromCopyQueue(SQLMsg ** pp)
{
	if (m_queue_query_copy.empty())
		return false;

	*pp = m_queue_query_copy.front();
	return true;
}

int32_t CAsyncSQL::CopyQuery()
{
	MUTEX_LOCK(m_mtxQuery);

	if (m_queue_query.empty())
	{
		MUTEX_UNLOCK(m_mtxQuery);
		return -1;
	}

	while (!m_queue_query.empty())
	{
		SQLMsg * p = m_queue_query.front();
		m_queue_query_copy.push(p);
		m_queue_query.pop();
	}

	int32_t count = m_queue_query_copy.size();	

	MUTEX_UNLOCK(m_mtxQuery);
	return count;
}

bool CAsyncSQL::PopQueryFromCopyQueue()
{
	if (m_queue_query_copy.empty())
	{
		return false;
	}

	m_queue_query_copy.pop();

	return true;
}
int32_t		CAsyncSQL::GetCopiedQueryCount()
{
	return m_iCopiedQuery;
}
void 	CAsyncSQL::ResetCopiedQueryCount()
{
	m_iCopiedQuery = 0;
}

void	CAsyncSQL::AddCopiedQueryCount(int32_t iCopiedQuery)
{
	m_iCopiedQuery += iCopiedQuery;
}



uint32_t CAsyncSQL::CountQuery()
{
	return m_queue_query.size();
}

uint32_t CAsyncSQL::CountResult()
{
	return m_queue_result.size();
}

void __timediff(struct timeval *a, struct timeval *b, struct timeval *rslt)
{
	if (a->tv_sec < b->tv_sec)
		rslt->tv_sec = rslt->tv_usec = 0;
	else if (a->tv_sec == b->tv_sec)
	{
		if (a->tv_usec < b->tv_usec)
			rslt->tv_sec = rslt->tv_usec = 0;
		else
		{
			rslt->tv_sec = 0;
			rslt->tv_usec = a->tv_usec - b->tv_usec;
		}
	}
	else
	{
		rslt->tv_sec = a->tv_sec - b->tv_sec;

		if (a->tv_usec < b->tv_usec)
		{
			rslt->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
			rslt->tv_sec--;
		} else
			rslt->tv_usec = a->tv_usec - b->tv_usec;
	}
}

class cProfiler
{
	public:
		cProfiler() 
		{
			m_nInterval = 0 ;

			memset( &prev, 0, sizeof(prev) );
			memset( &now, 0, sizeof(now) );
			memset( &interval, 0, sizeof(interval) );

			Start(); 
		}

		cProfiler(int32_t nInterval = 100000)
		{
			m_nInterval = nInterval;

			memset( &prev, 0, sizeof(prev) );
			memset( &now, 0, sizeof(now) );
			memset( &interval, 0, sizeof(interval) );

			Start(); 
		}

		void Start()
		{
			gettimeofday (&prev , (struct timezone *) 0);
		}

		void Stop()
		{
			gettimeofday(&now, (struct timezone*) 0); 
			__timediff(&now, &prev, &interval);
		}

		bool IsOk()
		{ 
			if (interval.tv_sec > (m_nInterval / 1000000))
				return false;

			if (interval.tv_usec > m_nInterval)
				return false;

			return true;
		}

		struct timeval * GetResult() { return &interval; }
		int32_t GetResultSec() { return interval.tv_sec; }
		int32_t GetResultUSec() { return interval.tv_usec; }

	private:
		int32_t m_nInterval;
		struct timeval  prev;
		struct timeval  now;
		struct timeval	interval;
};

void CAsyncSQL::ChildLoop()
{
	cProfiler profiler(500000);

	while (!m_bEnd)
	{
		m_sem.Wait();

		int32_t count = CopyQuery();

		if (count <= 0)
			continue;

		AddCopiedQueryCount(count);

		SQLMsg * p;

		while (count--)
		{
			profiler.Start();

			if (!PeekQueryFromCopyQueue(&p))
				continue;

			if (m_ulThreadID != mysql_thread_id(&m_hDB))
			{
				sys_log(0, "MySQL connection was reconnected. querying locale set"); // @correction011
				while (!QueryLocaleSet());
				m_ulThreadID = mysql_thread_id(&m_hDB);
			}

			if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
			{
				p->uiSQLErrno = mysql_errno(&m_hDB);

				sys_err("AsyncSQL: query failed: %s (query: %s errno: %d)", 
						mysql_error(&m_hDB), p->stQuery.c_str(), p->uiSQLErrno);

				switch (p->uiSQLErrno)
				{
					case CR_SOCKET_CREATE_ERROR:
					case CR_CONNECTION_ERROR:
					case CR_IPSOCK_ERROR:
					case CR_UNKNOWN_HOST:
					case CR_SERVER_GONE_ERROR:
					case CR_CONN_HOST_ERROR:
					case ER_NOT_KEYFILE:
					case ER_CRASHED_ON_USAGE:
					case ER_CANT_OPEN_FILE:
					case ER_HOST_NOT_PRIVILEGED:
					case ER_HOST_IS_BLOCKED:
					case ER_PASSWORD_NOT_ALLOWED:
					case ER_PASSWORD_NO_MATCH:
					case ER_CANT_CREATE_THREAD:
					case ER_INVALID_USE_OF_NULL:
						m_sem.Release();
						sys_err("AsyncSQL: retrying");
						continue;
				}
			}

			profiler.Stop();
			
			if (!profiler.IsOk())
				sys_log(0, "[QUERY : LONG INTERVAL(OverSec %ld.%ld)] : %s", 
						profiler.GetResultSec(), profiler.GetResultUSec(), p->stQuery.c_str());

			PopQueryFromCopyQueue();

			if (p->bReturn)
			{
				p->Store();
				PushResult(p);
			}
			else
				delete p;

			++m_iQueryFinished;
		}
	}

	SQLMsg * p;

	while (PeekQuery(&p))
	{
		if (m_ulThreadID != mysql_thread_id(&m_hDB))
		{
			sys_log(0, "MySQL connection was reconnected. querying locale set"); // @correction011
			while (!QueryLocaleSet());
			m_ulThreadID = mysql_thread_id(&m_hDB);
		}

		if (mysql_real_query(&m_hDB, p->stQuery.c_str(), p->stQuery.length()))
		{
			p->uiSQLErrno = mysql_errno(&m_hDB);

			sys_err("AsyncSQL::ChildLoop : mysql_query error: %s:\nquery: %s",
					mysql_error(&m_hDB), p->stQuery.c_str());

			switch (p->uiSQLErrno)
			{
				case CR_SOCKET_CREATE_ERROR:
				case CR_CONNECTION_ERROR:
				case CR_IPSOCK_ERROR:
				case CR_UNKNOWN_HOST:
				case CR_SERVER_GONE_ERROR:
				case CR_CONN_HOST_ERROR:
				case ER_NOT_KEYFILE:
				case ER_CRASHED_ON_USAGE:
				case ER_CANT_OPEN_FILE:
				case ER_HOST_NOT_PRIVILEGED:
				case ER_HOST_IS_BLOCKED:
				case ER_PASSWORD_NOT_ALLOWED:
				case ER_PASSWORD_NO_MATCH:
				case ER_CANT_CREATE_THREAD:
				case ER_INVALID_USE_OF_NULL:
					continue;
			}
		}

		sys_log(0, "QUERY_FLUSH: %s", p->stQuery.c_str());

		PopQuery(p->iID);

		if (p->bReturn)
		{
			p->Store();
			PushResult(p);
		}
		else
			delete p;

		++m_iQueryFinished;
	}
}

int32_t CAsyncSQL::CountQueryFinished()
{
	return m_iQueryFinished;
}

void CAsyncSQL::ResetQueryFinished()
{
	m_iQueryFinished = 0;
}

MYSQL * CAsyncSQL::GetSQLHandle()
{
	return &m_hDB;
}

size_t CAsyncSQL::EscapeString(char* dst, size_t dstSize, const char *src, size_t srcSize)
{
	if (0 == srcSize)
	{
		memset(dst, 0, dstSize);
		return 0;
	}

	if (0 == dstSize)
		return 0;

	if (dstSize < srcSize * 2 + 1)
	{
		char tmp[256];
		size_t tmpLen = sizeof(tmp) > srcSize ? srcSize : sizeof(tmp);
		strlcpy(tmp, src, tmpLen);

		sys_err("FATAL ERROR!! not enough buffer size (dstSize %u srcSize %u src%s: %s)",
				dstSize, srcSize, tmpLen != srcSize ? "(trimmed to 255 characters)" : "", tmp);

		dst[0] = '\0';
		return 0;
	}

	return mysql_real_escape_string(GetSQLHandle(), dst, src, srcSize);
}

void CAsyncSQL2::SetLocale(const std::string & stLocale)
{
	m_stLocale = stLocale;	
	QueryLocaleSet();
}

