#ifndef ZEN_CLIENT_ZDND_VIEWER_H
#define ZEN_CLIENT_ZDND_VIEWER_H

class ViewerInterface
{
 public:
  virtual ~ViewerInterface(){};
  virtual bool Show() { return true; }
};

#endif  //  ZEN_CLIENT_ZDND_VIEWER_H
