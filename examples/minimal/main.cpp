#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "QRetro.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("QRetro Example");

  auto *layout = new QVBoxLayout(&window);

  auto *corePathEdit = new QLineEdit(&window);
  corePathEdit->setPlaceholderText("Core path (.so/.dll/.dylib)");

  auto *contentPathEdit = new QLineEdit(&window);
  contentPathEdit->setPlaceholderText("Content path (optional)");

  auto *createButton = new QPushButton("Create", &window);

  layout->addWidget(corePathEdit);
  layout->addWidget(contentPathEdit);
  layout->addWidget(createButton);

  QObject::connect(createButton, &QPushButton::clicked, &window, [&]() {
    const QString corePath = corePathEdit->text().trimmed();
    const QString contentPath = contentPathEdit->text().trimmed();

    if (corePath.isEmpty())
      return;
 
    auto *retro = new QRetro();
    if (!retro->loadCore(corePath.toUtf8().constData()))
    {
      delete retro;
      return;
    }

    if (!contentPath.isEmpty())
    {
      if (!retro->loadContent(contentPath.toUtf8().constData()))
      {
        delete retro;
        return;
      }
    }

    if (!retro->startCore())
    {
      delete retro;
      return;
    }

    retro->setTitle("QRetro");
    retro->show();
  });

  window.resize(520, 120);
  window.show();
  return app.exec();
}
