/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef DATAREADERTHREAD_H
#define DATAREADERTHREAD_H

#include "../base/xthread.h"

class CeleX4;
class DataReaderThread : public XThread
{
public:
    DataReaderThread(CeleX4* pSensor, const std::string& name = "DataReaderThread");
    ~DataReaderThread();

    void startReadData(bool bRead);

protected:
    void run() override;

private:
    bool    m_bPipeoutAllowed;
	CeleX4* m_pCelexSensor;
};

#endif // DATAREADERTHREAD_H
