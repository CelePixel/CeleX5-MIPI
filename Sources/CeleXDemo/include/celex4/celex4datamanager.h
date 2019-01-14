#ifndef CELEX_DATA_MANAGER_H
#define CELEX_DATA_MANAGER_H

#include "celex4processeddata.h"

class CeleX4DataManager
{
public:
	enum emDataType {
		Default = 0,
		CeleX_Frame_Data = 1
	};

	CeleX4DataManager() { }
	~CeleX4DataManager() { }

	virtual void onFrameDataUpdated(CeleX4ProcessedData* data) = 0;
};

class CX4Subject
{
public:
	virtual void registerData(CeleX4DataManager* observer, CeleX4DataManager::emDataType type) = 0;
	virtual void unregisterData(CeleX4DataManager* observer, CeleX4DataManager::emDataType type) = 0;
	virtual void notify(CeleX4DataManager::emDataType dataType) = 0;
};

class CX4SensorDataServer : public CX4Subject
{
public:
	CX4SensorDataServer() : m_pObserver(NULL), m_pCX4ProcessedData(NULL)
	{
	}
	virtual ~CX4SensorDataServer()
	{
		if (m_pObserver)
			delete m_pObserver;
	}

	void registerData(CeleX4DataManager* observer, CeleX4DataManager::emDataType type)
	{
		m_pObserver = observer;
		m_listDataType.push_back(type);
	}

	void unregisterData(CeleX4DataManager* observer, CeleX4DataManager::emDataType type)
	{
		if (observer == m_pObserver)
		{
			m_listDataType.remove(type);
		}
	}

	void notify(CeleX4DataManager::emDataType dataType)
	{
		if (m_pObserver)
		{
			if (CeleX4DataManager::CeleX_Frame_Data == dataType)
				m_pObserver->onFrameDataUpdated(m_pCX4ProcessedData);
		}
	}
	inline void setCX4SensorData(CeleX4ProcessedData* data) { m_pCX4ProcessedData = data; }
	inline CeleX4ProcessedData* getCX4SensorData() { return m_pCX4ProcessedData; }

private:
	std::list<CeleX4DataManager::emDataType> m_listDataType;
	CeleX4DataManager*      m_pObserver;
	CeleX4ProcessedData*    m_pCX4ProcessedData;
};

#endif // CELEX_DATA_MANAGER_H
