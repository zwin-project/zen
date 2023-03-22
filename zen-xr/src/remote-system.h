#include "xr.h"
#include "zen-xr/system.h"

namespace zen::xr {

class RemoteSystem : public System
{
 public:
  DISABLE_MOVE_AND_COPY(RemoteSystem);
  explicit RemoteSystem(uint64_t handle, uint64_t peer_id);
  ~RemoteSystem() override = default;

  uint64_t handle() override;
  Type type() override;
  [[nodiscard]] inline uint64_t peer_id() const;

 private:
  const uint64_t handle_;
  const uint64_t peer_id_;
};

inline uint64_t
RemoteSystem::peer_id() const
{
  return peer_id_;
}

}  // namespace zen::xr
