#ifndef FTINTC_H_
#define FTINTC_H_


#define IRQ_SOURCE                      0
#define IRQ_MASK                        0x04
#define IRQ_CLEAR                       0x08
#define IRQ_MODE                        0x0c
#define IRQ_LEVEL                       0x10
#define IRQ_STATUS                      0x14

#define FIQ_SOURCE                      0x20
#define FIQ_MASK                        0x24
#define FIQ_CLEAR                       0x28
#define FIQ_MODE                        0x2c
#define FIQ_LEVEL                       0x30
#define FIQ_STATUS                      0x34

#define FIQ_OFFSET                      0


#define EIRQ_SOURCE				0x60
#define EIRQ_MASK				0x64
#define EIRQ_CLEAR				0x68
#define EIRQ_MODE				0x6c
#define EIRQ_LEVEL				0x70
#define EIRQ_STATUS				0x74

#define EFIQ_SOURCE                      0x80
#define EFIQ_MASK                        0x84
#define EFIQ_CLEAR                       0x88
#define EFIQ_MODE                        0x8c
#define EFIQ_LEVEL                       0x90
#define EFIQ_STATUS                      0x94



#endif
