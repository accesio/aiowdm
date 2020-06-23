enum  REGISTER_ACTION
{
REGISTER_ACTION_NONE,
REGISTER_ACTION_READ_8,
REGISTER_ACTION_READ_16,
REGISTER_ACTION_READ_32,						
REGISTER_ACTION_WRITE_8,
REGISTER_ACTION_WRITE_16,
REGISTER_ACTION_WRITE_32,
REGISTER_ACTION_WRITE_48,			// write a register
REGISTER_ACTION_TESTBITS_8,
REGISTER_ACTION_TESTBITS_16,
REGISTER_ACTION_TESTBITS_32,			// reads a register and returns true if the masked bits' pattern matches
REGISTER_ACTION_WRITEBITS_8,	
REGISTER_ACTION_WRITEBITS_16,
REGISTER_ACTION_WRITEBITS_32,		// modifies some bits in a register (usually via read-modify-write operation)
REGISTER_ACTION_TOGGLEBITS_8,
REGISTER_ACTION_TOGGLEBITS_16,
REGISTER_ACTION_TOGGLEBITS_32,	// toggle a bit (or some bits) in a register (via read-modify-writeusually)
};
	
// LOCAL const char * eActionNames[] = 
// 					{"Not Applicable",
// 					 "READ8","READ16","READ32",
// 					 "WRITE8","WRITE16","WRITE32","WRITE48",
// 					 "TESTBITS8","TESTBITS16","TESTBITS32",
// 					 "WRITEBITS8","WRITEBITS16","WRITEBITS32",
// 					 "TOGGLEBITS8","TOGGLEBITS16","TOGGLEBITS32",
// 					 "NYI","ERROR_PARSING",
// 					};

// LOCAL const char * eActionFmt[]=
// 					{"%s [%15s]\n",
// 					 "%s [%15s @bar[%02hX]+%02X]\n","%s [%15s @bar[%02hX]+%02X]\n","[%15s @bar[%02hX]+%02X]\n",
// 					 "%s [%15s @bar[%02hX]+%02X, .v=%04lX]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX]\n", "%s [%15s @bar[%02hX]+%02X, .v=%04lX] %s\n", "[%15s @bar[%02hX]+%02X, .v=%04lX]\n",
// 					 "%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n",
// 					 "%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n",
// 					 "%s [%15s]\n","%s [%15s @bar[%02hX]+%02X, .v=%04lX, .m=%04lX]\n",
// 					};

//typedef void (*aioActionHandler)(TRegAct *action);

struct register_operation
{
    enum REGISTER_ACTION action;
    uint8_t  bar;
    uint16_t offset; 					// register index
    uint32_t value;
    uint32_t mask;					// which bits in Value should be asserted (for WRITEBITS) or tested (for TESTBITS)
//    aioActionHandler func;	//optional function to perform action
}; // TRegisterAction
