#ifndef SDI_IFACE_H
#define SDI_IFACE_H

#ifdef __amigaos4__
#define EXEC_INTERFACE_DECLARE(x) x
#define EXEC_INTERFACE_GET_MAIN(interface,libbase) (interface = (void*)GetInterface((struct Library*)(libbase),"main",1,NULL))
#define EXEC_INTERFACE_DROP(interface) DropInterface((struct Interface*)interface)
#define EXEC_INTERFACE_ASSIGN(a,b) (a = (b))
#else
#define EXEC_INTERFACE_DECLARE(x)
#define EXEC_INTERFACE_GET_MAIN(interface,libbase) 1
#define EXEC_INTERFACE_DROP(interface) do {} while(0);
#define EXEC_INTERFACE_ASSIGN(a,b) do {} while(0);
#endif

#endif
