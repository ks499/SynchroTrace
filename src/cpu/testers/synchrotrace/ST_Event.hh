/*Copyright (c) 2015, Drexel University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

/*
 * Authors: Karthik Sangaiah & Ankit More
 *
 * ST_Event.hh
 *
 *
 * Defines an event in the queue for synchronization and event-based dependency tracking
 *
 */
 
#ifndef __CPU_SYNCHROTRACE_STEVENT_HH__
#define __CPU_SYNCHROTRACE_STEVENT_HH__
 
#define TYPE_COMPUTATION        0
#define TYPE_COMMUNICATION      1
#define TYPE_PTHREAD_API        2

#define P_MUTEX_LK              1
#define P_MUTEX_ULK             2
#define P_CREATE                3
#define P_JOIN                  4
#define P_BARRIER_WT            5
#define P_COND_WT               6
#define P_COND_SG               7
#define P_SPIN_LK               8
#define P_SPIN_ULK              9
#define P_SEM_INIT              10
#define P_SEM_WAIT              11
#define P_SEM_POST              12
#define P_SEM_GETV              13
#define P_SEM_DEST              14

 
//#include <vector>
//#include <deque>
#include "cpu/testers/synchrotrace/lib/static_deque.hh"
#include <stdlib.h>
#include "mem/ruby/common/DataBlock.hh"
#include "mem/ruby/common/SubBlock.hh"
#include "mem/ruby/system/RubyPort.hh"
using namespace std;
 
// request type from profile
enum dtReqType { DT_REQ_INVALID, DT_REQ_READ, DT_REQ_WRITE, DT_REQ_LAST };

struct sharedInfo {
  public:
    unsigned int prodThreadID;
    unsigned long prodEventID;
    unsigned long addr;
    unsigned int numBytes;
 
    //Constructors
    sharedInfo() {}
    sharedInfo(const unsigned int pt_ID, const unsigned long pe_ID, const unsigned long la, const unsigned int nb) {
        prodThreadID = pt_ID;
        prodEventID = pe_ID;
        addr = la;
	numBytes = nb;
    }
    sharedInfo(const unsigned long la, const unsigned int nb) {
        prodThreadID = -1;
        prodEventID = -1;
        addr = la;
	numBytes = nb;
    }
};

struct subEvent {
    unsigned long numIOPS;
    unsigned long numFLOPS;
    unsigned long numMEM_OPS;
    dtReqType msgType;
    Time triggerTime;
    bool msgTriggered;
    bool containsMsg;
    sharedInfo* thisMsg;
    Time initialTriggerTime;
 
    //Constructors
    subEvent() {}
    subEvent(const unsigned long i, const unsigned long f, const unsigned long mem, const dtReqType ty, const bool mt, const bool cm, sharedInfo* se_ptr) {
        numIOPS = i;
        numFLOPS = f;
        numMEM_OPS = mem;
        msgType = ty;
        msgTriggered = mt;
        containsMsg = cm;
        thisMsg = se_ptr;
    }
 
    subEvent(const unsigned long i, const unsigned long f, const unsigned long mem, const bool mt, const bool cm) {
        numIOPS = i;
        numFLOPS = f;
        numMEM_OPS = mem;
        msgTriggered = mt;
        containsMsg = cm;
    }
};
 
struct threadStats {
    int ThreadID;
    unsigned long numSubEvents;
    unsigned long numIOPS;
    unsigned long numFLOPS;
    unsigned long localWrites;
    unsigned long localReads;
    unsigned long sharedWrites;
    unsigned long sharedReads;
    unsigned long totalRTT;
 
    threadStats() {
        ThreadID = -1;
        numSubEvents = 0;
        numIOPS = 0;
        numFLOPS = 0;
        localWrites = 0;
        localReads = 0;
        sharedWrites = 0;
        sharedReads = 0;
        totalRTT = 0;
    }
 
    void clear() {
        numSubEvents = 0;
        numIOPS = 0;
        numFLOPS = 0;
        localWrites = 0;
        localReads = 0;
        sharedWrites = 0;
        sharedReads = 0;
        totalRTT = 0;
    }
};
 
class ST_Event
{
  public:
    typedef int event_ty;

    event_ty Type, PthreadType;
    unsigned long EventID;
    int ThreadID;
 
    vector< sharedInfo* > comm_preRequisiteEvents;
 
    vector<sharedInfo* > comp_writeEvents;
    vector<sharedInfo* > comp_readEvents;
    unsigned long compIOPS;
    unsigned long compFLOPS;
    unsigned long compMem_reads;
    unsigned long compMem_writes;
    bool subEventsCreated;

    uint64_t pth_addr;
 
    StaticDeque< subEvent > *subEventList;
 
    ST_Event() {
        Type = -1;
        PthreadType = -1;
        EventID = -1;
        ThreadID = -1;
        compIOPS = -1;
        compFLOPS = -1;
        compMem_reads = -1;
        compMem_writes = -1;
        subEventsCreated = false;
    }
    ~ST_Event() {
      delete subEventList;
      for (vector<sharedInfo *>::iterator i = comm_preRequisiteEvents.begin(); i != comm_preRequisiteEvents.end(); i++)
        delete *i;
      for (vector<sharedInfo *>::iterator i = comp_writeEvents.begin(); i != comp_writeEvents.end(); i++)
        delete *i;
      for (vector<sharedInfo *>::iterator i = comp_readEvents.begin(); i != comp_readEvents.end(); i++)
        delete *i;
      vector<sharedInfo *>().swap(comm_preRequisiteEvents);
      vector<sharedInfo *>().swap(comp_writeEvents);
      vector<sharedInfo *>().swap(comp_readEvents);
    }
};
 
// Output overloading 
 
inline ostream & operator << (ostream & os, const ST_Event & thisEvent)
{
    os << "Type:" << thisEvent.Type;
    os << " EventID:" << thisEvent.EventID;
    os << " ThreadID:" << thisEvent.ThreadID;
 
    if (thisEvent.Type == TYPE_COMPUTATION){
        os << " compIOPS:" << thisEvent.compIOPS;
        os << " compFLOPS:" << thisEvent.compFLOPS;
        os << " compMem_reads:" << thisEvent.compMem_reads;
        os << " compMem_writes:" << thisEvent.compMem_writes;
    }
 
    os << " Sub-events creation status:" << thisEvent.subEventsCreated;
 
    return os;
}
 
inline ostream & operator << (ostream & os, const sharedInfo & thisSharedInfo)
{
    os << " prodThreadID:" << thisSharedInfo.prodThreadID;
    os << " prodEventID:" << thisSharedInfo.prodEventID;
    os << " addr:" << thisSharedInfo.addr;
    os << " numBytes:" << thisSharedInfo.numBytes;
 
    return os;
}
#endif
