#include <QApplication>
#include <math.h>
#include <iostream>

#include "MarxGL2Din3DSpaceGraph.h"
#include <QPushButton>
#include "TopPanelNew.h"

int main(int argc, char *argv[])
{

  QApplication app(argc, argv);

  if (argc < 2) {
    cout << "Usuage:\n\t$ ./Marx2DGraph /path/to/audiofile.au\n";
    return 0;
  }

  string progName = argv[0];
  string audioFile = argv[1];

  TopPanelNew* panel = new TopPanelNew( audioFile );
  
  panel->setGeometry(100, 10, 600, 600);
  panel->show();

  return app.exec();
}
