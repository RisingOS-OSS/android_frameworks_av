/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef A_RTP_SOURCE_H_

#define A_RTP_SOURCE_H_

#include <stdint.h>

#include <media/stagefright/foundation/ABase.h>
#include <utils/List.h>
#include <utils/RefBase.h>

namespace android {

struct ABuffer;
struct AMessage;
struct ARTPAssembler;
struct ASessionDescription;

struct ARTPSource : public RefBase {
    ARTPSource(
            uint32_t id,
            const sp<ASessionDescription> &sessionDesc, size_t index,
            const sp<AMessage> &notify);

    void processRTPPacket(const sp<ABuffer> &buffer);
    void timeUpdate(uint32_t rtpTime, uint64_t ntpTime);
    void byeReceived();

    List<sp<ABuffer> > *queue() { return &mQueue; }

    void addReceiverReport(const sp<ABuffer> &buffer);
    void addFIR(const sp<ABuffer> &buffer);
    void addTMMBR(const sp<ABuffer> &buffer);
    void setSelfID(const uint32_t selfID);
    void setMinMaxBitrate(int32_t min, int32_t max);

    bool isNeedToReport();

    void noticeAbandonBuffer(int cnt=1);

    int32_t mFirstSeqNumber;
    int32_t mFirstRtpTime;
    int64_t mFirstSysTime;
    int32_t mClockRate;

private:
    struct QualManager {
        QualManager() : mMinBitrate(-1), mMaxBitrate(-1), mTargetBitrate(-1) {};

        int32_t mMinBitrate;
        int32_t mMaxBitrate;
        int32_t mBitrateStep;

        int32_t mTargetBitrate;

        void setTargetBitrate(uint8_t fraction) {
            if (fraction <= (256 * 2 /100)) {           // loss less than 2%
                mTargetBitrate += mBitrateStep;
            } else if (fraction > (256 * 5 / 100)) {    // loss more than 5%
                mTargetBitrate -= mBitrateStep;
            }

            if (mTargetBitrate > mMaxBitrate)
                mTargetBitrate = mMaxBitrate;
            else if (mTargetBitrate < mMinBitrate)
                mTargetBitrate = mMinBitrate;
        };

        void setMinMaxBitrate(int32_t min, int32_t max) {
            mMinBitrate = min;
            mMaxBitrate = max;
            mBitrateStep = (max - min) / 8;
            mTargetBitrate = min;
        };
    } mQualManager;

    uint32_t mID;
    uint32_t mHighestSeqNumber;
    uint32_t mPrevExpected;
    uint32_t mBaseSeqNumber;
    int32_t mNumBuffersReceived;
    int32_t mPrevNumBuffersReceived;

    List<sp<ABuffer> > mQueue;
    sp<ARTPAssembler> mAssembler;

    uint64_t mLastNTPTime;
    int64_t mLastNTPTimeUpdateUs;

    bool mIssueFIRRequests;
    int64_t mLastFIRRequestUs;
    uint8_t mNextFIRSeqNo;

    sp<AMessage> mNotify;

    bool queuePacket(const sp<ABuffer> &buffer);

    DISALLOW_EVIL_CONSTRUCTORS(ARTPSource);
};

}  // namespace android

#endif  // A_RTP_SOURCE_H_
