#include <stdio.h>
#include <thread>
#include "aiowdm.h"

void GenIrq ()
{
  bool set = true;
  for (int i = 0 ; i < 3 ; i++)
  {
    printf("Setting DIO 0 to %d\n", set ? 1 : 0);
    AIOWDM::RelOutPortL(0, 0x44, set ? 0x1 : 0);
    printf("Reading 0x40: 0x%x\n", AIOWDM::RelInPortL(0, 0x40));
    printf("Reading 0x44: 0x%x\n", AIOWDM::RelInPortL(0, 0x44));
    printf("Reading 0x48: 0x%x\n", AIOWDM::RelInPortL(0, 0x48));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    set = !set;
  }
  AIOWDM::AbortRequest(0);
}

int main (int argc, char **)
{
  int NumCards = AIOWDM::GetNumCards();
  int Status;
  unsigned long DeviceId;
  std::thread IrqThread;

  if (NumCards == 0) {
    printf("No cards detected\n");
  }

  Status = AIOWDM::QueryCardInfo(0, &DeviceId, nullptr, nullptr, nullptr);

  if (DeviceId != 0xc2ef)
  {
    printf("Development test only to be used with 0xc2ef device id\n");
    return -1;
  }

  //Writing 1 to address 0 resets card to power on state
  Status = AIOWDM::RelOutPortB(0, 0, 1);

  if (Status)
  {
    printf("Failed to reset card\n");
    return -1;
  }

  //Enable IRQ on DIO 13
  //set group 0 to be output so we can use it to generate the IRQ on DIO 13
  Status = AIOWDM::RelOutPortL(0, 0x48, 0x400001);

  if (Status)
  {
    printf("Failed to enable external IRQ\n");
    return -1;
  }

  //Enabel enExt0
  Status= AIOWDM::RelOutPortL(0, 0x40, 0x0080);

    if (Status)
  {
    printf("Failed to enable external IRQ\n");
    return -1;
  }

  IrqThread = std::thread(GenIrq);

  Status = AIOWDM::WaitForIRQ(0);

  printf ("Status of WaitForIRQ = %d\n", Status);
  IrqThread.join();
}

