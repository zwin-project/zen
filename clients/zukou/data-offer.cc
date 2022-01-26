#include <fcntl.h>
#include <unistd.h>
#include <zigen-client-protocol.h>
#include <zukou.h>

namespace zukou {

static void
data_offer_offer(void *data,
    [[maybe_unused]] struct zgn_data_offer *zgn_data_offer,
    const char *mime_type)
{
  DataOffer *data_offer = (DataOffer *)data;
  data_offer->Offer(mime_type);
}

static void
data_offer_source_actions(void *data,
    [[maybe_unused]] struct zgn_data_offer *zgn_data_offer,
    uint32_t source_actions)
{
  DataOffer *data_offer = (DataOffer *)data;
  data_offer->SourceActions(source_actions);
}

static void
data_offer_action(void *data,
    [[maybe_unused]] struct zgn_data_offer *zgn_data_offer, uint32_t dnd_action)
{
  DataOffer *data_offer = (DataOffer *)data;
  data_offer->Action(dnd_action);
}

static const struct zgn_data_offer_listener data_offer_listener = {
    data_offer_offer,
    data_offer_source_actions,
    data_offer_action,
};

DataOffer::DataOffer(struct zgn_data_offer *data_offer, App *app)
    : app_(app), data_offer_(data_offer)
{
  zgn_data_offer_add_listener(data_offer, &data_offer_listener, this);
}

DataOffer::~DataOffer() { zgn_data_offer_destroy(data_offer_); }

void
DataOffer::Offer(const char *mime_type)
{
  mime_types_.push_back(mime_type);
}

void
DataOffer::SourceActions(uint32_t source_actions)
{
  source_actions_ = source_actions;
}

void
DataOffer::Action(uint32_t action)
{
  dnd_action_ = action;
}

void
DataOffer::Receive(std::string mime_type, DndTask *task)
{
  int p[2];
  struct epoll_event ep;

  if (pipe2(p, O_CLOEXEC) == -1) return;

  zgn_data_offer_receive(this->data_offer(), mime_type.c_str(), p[1]);
  close(p[1]);

  task->fd = p[0];

  ep.events = EPOLLIN;
  ep.data.ptr = task;

  epoll_ctl(app_->epoll_fd(), EPOLL_CTL_ADD, p[0], &ep);
}

void
DataOffer::Accept(uint32_t serial, std::string mime_type)
{
  zgn_data_offer_accept(data_offer(), serial, mime_type.c_str());
}

void
DataOffer::SetActions(uint32_t actions, uint32_t preffered_action)
{
  zgn_data_offer_set_actions(data_offer(), actions, preffered_action);
}

}  // namespace zukou
