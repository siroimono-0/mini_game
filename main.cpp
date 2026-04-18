#include "Support.h"
#include "UI.h"

using namespace std;

int main()
{
  try
  {
    Ch ch;
    Sv sv(ch);
    sv.open_s();
    UI ui(sv);
    ui.run();
  }
  catch (Exception err)
  {
    printf("err type == %s\n"
           "err name == %s\n"
           "err no ==%d\n",
           err.get_err_type().c_str(), err.get_err_name().c_str(),
           err.get_err_no());
    return 0;
  }
  return 0;
}