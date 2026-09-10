#define SET_BIT(REG , pin) ( (REG) |= (1 << (pin)) ) 
#define TOG_BIT(reg , pin) ( (reg) ^= (1 << (pin)) ) 
#define CLR_BIT(reg , pin) ( (reg) &= ~(1 << (pin)) )
#define GET_BIT(reg , pin) ( ((reg) & (1 << (pin))) >> (pin) )