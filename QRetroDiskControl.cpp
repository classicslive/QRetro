#include "QRetroDiskControl.h"
#include "QRetro.h"

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
  bool retval = false;

  if (!m_Callback.set_eject_state)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.set_eject_state(ejected); });

  return retval;
}

bool QRetroDiskControl::getEjectState(void)
{
  bool retval = false;

  if (!m_Callback.get_eject_state)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.get_eject_state(); });

  return retval;
}

unsigned QRetroDiskControl::getImageIndex(void)
{
  unsigned retval = 0;

  if (!m_Callback.get_image_index)
    return 0;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.get_image_index(); });

  return retval;
}

bool QRetroDiskControl::setImageIndex(unsigned index)
{
  bool retval = false;

  if (!m_Callback.set_image_index)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.set_image_index(index); });

  return retval;
}

unsigned QRetroDiskControl::getNumImages(void)
{
  unsigned retval = 0;

  if (!m_Callback.get_num_images)
    return 0;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.get_num_images(); });

  return retval;
}

bool QRetroDiskControl::replaceImageIndex(unsigned index, const retro_game_info *info)
{
  bool retval = false;

  if (!m_Callback.replace_image_index)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.replace_image_index(index, info); });

  return retval;
}

bool QRetroDiskControl::addImageIndex(void)
{
  bool retval = false;

  if (!m_Callback.add_image_index)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.add_image_index(); });

  return retval;
}

bool QRetroDiskControl::setInitialImage(unsigned index, const char *path)
{
  bool retval = false;

  if (m_Version != QRetroDiskControl::v1 || !m_Callback.set_initial_image)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.set_initial_image(index, path); });

  return retval;
}

bool QRetroDiskControl::getImagePath(unsigned index, char *s, size_t len)
{
  bool retval = false;

  if (m_Version != QRetroDiskControl::v1 || !m_Callback.get_image_path)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.get_image_path(index, s, len); });

  return retval;
}

bool QRetroDiskControl::getImageLabel(unsigned index, char *s, size_t len)
{
  bool retval = false;

  if (m_Version != QRetroDiskControl::v1 || !m_Callback.get_image_label)
    return false;
  m_owner->execOnTimingThread([&]() { retval = m_Callback.get_image_label(index, s, len); });

  return retval;
}
