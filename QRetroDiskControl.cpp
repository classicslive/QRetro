#include "QRetroDiskControl.h"

bool QRetroDiskControl::setInterface(const retro_disk_control_callback *callback)
{
  if (!callback)
    return false;
  else
  {
    m_Callback.set_eject_state = callback->set_eject_state;
    m_Callback.get_eject_state = callback->get_eject_state;
    m_Callback.get_image_index = callback->get_image_index;
    m_Callback.set_image_index = callback->set_image_index;
    m_Callback.get_num_images = callback->get_num_images;
    m_Callback.replace_image_index = callback->replace_image_index;
    m_Callback.add_image_index = callback->add_image_index;
    m_Callback.set_initial_image = nullptr;
    m_Callback.get_image_path = nullptr;
    m_Callback.get_image_label = nullptr;
    m_Version = QRetroDiskControl::v0;

    return true;
  }
}

bool QRetroDiskControl::setExtInterface(const retro_disk_control_ext_callback *callback)
{
  if (!callback)
    return false;
  else
  {
    m_Callback = *callback;
    m_Version = QRetroDiskControl::v1;

    return true;
  }
}

bool QRetroDiskControl::setEjectState(bool ejected)
{
  if (!m_Callback.set_eject_state)
    return false;
  return m_Callback.set_eject_state(ejected);
}

bool QRetroDiskControl::getEjectState(void)
{
  if (!m_Callback.get_eject_state)
    return false;
  return m_Callback.get_eject_state();
}

unsigned QRetroDiskControl::getImageIndex(void)
{
  if (!m_Callback.get_image_index)
    return 0;
  return m_Callback.get_image_index();
}

bool QRetroDiskControl::setImageIndex(unsigned index)
{
  if (!m_Callback.set_image_index)
    return false;
  return m_Callback.set_image_index(index);
}

unsigned QRetroDiskControl::getNumImages(void)
{
  if (!m_Callback.get_num_images)
    return 0;
  return m_Callback.get_num_images();
}

bool QRetroDiskControl::replaceImageIndex(unsigned index, const retro_game_info *info)
{
  if (!m_Callback.replace_image_index)
    return false;
  return m_Callback.replace_image_index(index, info);
}

bool QRetroDiskControl::addImageIndex(void)
{
  if (!m_Callback.add_image_index)
    return false;
  return m_Callback.add_image_index();
}

bool QRetroDiskControl::setInitialImage(unsigned index, const char *path)
{
  if (m_Version != QRetroDiskControl::v1 || !m_Callback.set_initial_image)
    return false;
  return m_Callback.set_initial_image(index, path);
}

bool QRetroDiskControl::getImagePath(unsigned index, char *s, size_t len)
{
  if (m_Version != QRetroDiskControl::v1 || !m_Callback.get_image_path)
    return false;
  return m_Callback.get_image_path(index, s, len);
}

bool QRetroDiskControl::getImageLabel(unsigned index, char *s, size_t len)
{
  if (m_Version != QRetroDiskControl::v1 || !m_Callback.get_image_label)
    return false;
  return m_Callback.get_image_label(index, s, len);
}
