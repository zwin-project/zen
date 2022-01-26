#include <zigen-client-protocol.h>
#include <zukou.h>

namespace zukou {

static void
data_source_target(void *data,
    [[maybe_unused]] struct zgn_data_source *zgn_data_source,
    const char *mime_type)
{
  DataSource *data_source = (DataSource *)data;
  data_source->Target(mime_type);
}

static void
data_source_send(void *data,
    [[maybe_unused]] struct zgn_data_source *zgn_data_source,
    const char *mime_type, int32_t fd)
{
  DataSource *data_source = (DataSource *)data;
  data_source->Send(mime_type, fd);
}

static void
data_source_cancelled(
    void *data, [[maybe_unused]] struct zgn_data_source *zgn_data_source)
{
  DataSource *data_source = (DataSource *)data;
  data_source->Cancelled();
}

static void
data_source_dnd_drop_performed(
    void *data, [[maybe_unused]] struct zgn_data_source *zgn_data_source)
{
  DataSource *data_source = (DataSource *)data;
  data_source->DndDropPerformed();
}

static void
data_source_dnd_finished(
    void *data, [[maybe_unused]] struct zgn_data_source *zgn_data_source)
{
  DataSource *data_source = (DataSource *)data;
  data_source->DndFinished();
}

static void
data_source_action(void *data,
    [[maybe_unused]] struct zgn_data_source *zgn_data_source,
    uint32_t dnd_action)
{
  DataSource *data_source = (DataSource *)data;
  data_source->Action(dnd_action);
}

static const struct zgn_data_source_listener data_source_listener = {
    data_source_target,
    data_source_send,
    data_source_cancelled,
    data_source_dnd_drop_performed,
    data_source_dnd_finished,
    data_source_action,
};

DataSource::DataSource(App *app) : app_(app)
{
  data_source_ =
      zgn_data_device_manager_create_data_source(app->data_device_manager());

  zgn_data_source_add_listener(data_source_, &data_source_listener, this);
}

DataSource::~DataSource() { zgn_data_source_destroy(data_source_); }

void
DataSource::Target(const char *mime_type)
{
  app_->DataSourceTarget(mime_type);
}

void
DataSource::Send(const char *mime_type, int32_t fd)
{
  app_->DataSourceSend(mime_type, fd);
}

void
DataSource::Cancelled()
{
  app_->DestroyDataSource();
}

void
DataSource::DndDropPerformed()
{
  app_->DataSourceDndDropPerformed();
}

void
DataSource::DndFinished()
{
  app_->DestroyDataSource();
}

void
DataSource::Action(uint32_t dnd_action)
{
  app_->DataSourceAction(dnd_action);
}

}  // namespace zukou
