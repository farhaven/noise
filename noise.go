package main;

import (
    "unsafe"
    "os"
    "fmt"
    "syscall"
)

const (
    SIOC_OUT      = 0x20000000
    SIOC_IN       = 0x40000000
    SIOC_INOUT    = (SIOC_IN|SIOC_OUT)
    SIOCPARM_MASK = 0x1fff

    IOC_NRBITS = 8
    IOC_TYPEBITS = 8
    IOC_SIZEBITS = 14
    IOC_DIRBITS = 2

    IOC_NRMASK = (1 << IOC_NRBITS) - 1
    IOC_TYPEMASK = (1 << IOC_TYPEBITS) - 1
    IOC_SIZEMASK = (1 << IOC_SIZEBITS) - 1
    IOC_DIRMASK = (1 << IOC_DIRBITS) - 1

    IOC_NRSHIFT = 0
    IOC_TYPESHIFT = IOC_NRSHIFT + IOC_NRBITS
    IOC_SIZESHIFT = IOC_TYPESHIFT + IOC_TYPEBITS
    IOC_DIRSHIFT = IOC_SIZESHIFT + IOC_SIZEBITS

    IOC_NONE = 0
    IOC_WRITE = 1
    IOC_READ = 2
)

func IOWR(x uint32, y uint32, t uint32) uintptr {
    return uintptr(SIOC_INOUT|((uint32(unsafe.Sizeof(t)) & SIOCPARM_MASK) <<16 ) | (x<<8) | y)
}

type sounddev struct {
    dev string
    fh *(os.File)
    write_bits uint32
    write_channels uint32
    write_rate uint32
}

func (p *sounddev) Setup () (os.Error) {
    var err os.Error
    p.fh, err = os.Open(p.dev, os.O_WRONLY, 0)
    if err != nil {
        return err
    }

    syscall.Syscall(syscall.SYS_IOCTL, uintptr(p.fh.Fd()), IOWR('P', 0x05, 1), uintptr(unsafe.Pointer(&(p.write_bits))))
    syscall.Syscall(syscall.SYS_IOCTL, uintptr(p.fh.Fd()), IOWR('P', 0x06, 1), uintptr(unsafe.Pointer(&(p.write_channels))))
    syscall.Syscall(syscall.SYS_IOCTL, uintptr(p.fh.Fd()), IOWR('P', 0x02, 1), uintptr(unsafe.Pointer(&(p.write_rate))))

    return err
}

func write_noise(sd sounddev, buf []byte, ch chan bool) {
    fmt.Printf("enter write_noise\n")
    defer fmt.Printf("leave write_noise\n")

    for {
        <-ch
        var n int
        var err os.Error
        n, err = sd.fh.Write(buf)
        if n == 0 && err != nil {
            fmt.Fprintf(os.Stderr, "%v\n", err)
        }
    }
}

func main() {
    var sd sounddev
    sd.dev = "/dev/dsp"
    sd.write_bits = 8
    sd.write_channels = 2
    sd.write_rate = 1000 * 16

    err := sd.Setup()
    if err != nil {
        fmt.Fprintf(os.Stderr, "%v\n", err)
        os.Exit(-1)
    }

    buf := make([]byte, 8)
    ch := make(chan bool)
    go write_noise(sd, buf, ch)

    for {
        ch <- true
        buf[0] = 0x00
        buf[1] = 0x00

        buf[2] = 0x00
        buf[3] = 0xFF
    }
}
