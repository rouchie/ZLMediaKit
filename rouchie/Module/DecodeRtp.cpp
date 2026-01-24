#include "DecodeRtp.h"

bool FrameWriterImp::inputFrame(const Frame::Ptr& frame)
{
    InfoL << frame;
    return true;
}

RtpDecodeMod::RtpDecodeMod()
    : RQModuleBase(0)
{
    _demuxer = std::make_shared<RtspDemuxer>();
    _demuxer->setTrackListener(this);

    _frame_writer = std::make_shared<FrameWriterImp>();
}

RtpDecodeMod::~RtpDecodeMod()
{
}

int RtpDecodeMod::OnStart()
{
    // 这里可以从配置文件或者其他途径获取sdp信息
    std::string sdp;
    _demuxer->loadSdp(sdp);

    return 0;
}

void RtpDecodeMod::onRtpSorted(RtpPacket::Ptr rtp, int index)
{
    bool key_pos = _demuxer->inputRtp(rtp);
}

bool RtpDecodeMod::addTrack(const Track::Ptr& track)
{
    track->addDelegate(_frame_writer);
    return true;
}

void RtpDecodeMod::addTrackCompleted()
{

}

void RtpDecodeMod::resetTracks()
{

}

class A {

};

class D {

};

class B : public D {

};

class C : public A, public B, public std::enable_shared_from_this<C> {
};

void a()
{
    std::shared_ptr<C> c = std::make_shared<C>();
    std::shared_ptr<D> b = std::static_pointer_cast<D>(c);
}
