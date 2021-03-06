#include "syscalls.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
static constexpr uint32_t G_SHMEM_BASE = 0x70000000;
static const uint32_t sbrk_start = 0x40000000;
static const uint32_t sbrk_max   = sbrk_start + 0x1000000;
static const uint32_t heap_start = sbrk_max;
static const uint32_t heap_max   = 0xF0000000;

struct iovec32 {
    uint32_t iov_base;
    int32_t  iov_len;
};

template <>
uint32_t syscall_openat<4>(riscv::Machine<4>& machine)
{
	const int fd = machine.sysarg<int>(0);
    if constexpr (verbose_syscalls) {
		printf("SYSCALL openat called, fd = %d  \n", fd);
	}
    return -EBADF;
}

template <>
uint32_t syscall_readlinkat<4>(riscv::Machine<4>& machine)
{
	const int fd = machine.sysarg<int>(0);
    if constexpr (verbose_syscalls) {
		printf("SYSCALL readlinkat called, fd = %d  \n", fd);
	}
    return -EBADF;
}

template <>
uint32_t syscall_write<4>(riscv::Machine<4>& machine)
{
	const int  fd      = machine.sysarg<int>(0);
	const auto address = machine.sysarg<uint32_t>(1);
	const auto len     = machine.sysarg<size_t>(2);
	if constexpr (false) {
		printf("SYSCALL write called, addr = %#X  len = %zu\n", address, len);
	}
	// we only accept standard pipes, for now :)
	if (fd >= 0 && fd < 3) {
		uint8_t buffer[len];
		machine.memory.memcpy_out(buffer, address, len);
		return write(fd, buffer, len);
	}
	return -EBADF;
}

template <>
uint32_t syscall_writev<4>(riscv::Machine<4>& machine)
{
	const int  fd     = machine.sysarg<int>(0);
	const auto iov_g  = machine.sysarg<uint32_t>(1);
	const auto count  = machine.sysarg<int>(2);
	if constexpr (false) {
		printf("SYSCALL writev called, iov = %#X  cnt = %d\n", iov_g, count);
	}
	if (count < 0 || count > 256) return -EINVAL;
	// we only accept standard pipes, for now :)
	if (fd >= 0 && fd < 3) {
        const size_t size = sizeof(iovec32) * count;

        std::vector<iovec32> vec(count);
        machine.memory.memcpy_out(vec.data(), iov_g, size);

        int res = 0;
        for (const auto& iov : vec)
        {
			char buffer[1024];
            auto src_g = (uint32_t) iov.iov_base;
            auto len_g = std::min(sizeof(buffer), (size_t) iov.iov_len);
            machine.memory.memcpy_out(buffer, src_g, len_g);
            res += write(fd, buffer, len_g);
        }
        return res;
	}
	return -EBADF;
}

template <>
uint32_t syscall_mmap<4>(riscv::Machine<4>& machine)
{
	const int  addr_g = machine.sysarg<uint32_t>(0);
	const auto length = machine.sysarg<uint32_t>(1);
	const auto prot   = machine.sysarg<int>(2);
    const auto flags  = machine.sysarg<int>(3);
	if constexpr (verbose_syscalls) {
		printf("SYSCALL mmap called, addr %#X  len %u prot %#x flags %#X\n",
                addr_g, length, prot, flags);
	}
    if (addr_g == 0 && (length % riscv::Page::size()) == 0)
    {
        static uint32_t nextfree = heap_start;
        const uint32_t addr = nextfree;
        //auto& page = machine.memory.create_page(addr);
        //page.attr.read = prot & PROT_READ;
        nextfree += length;
        return addr;
    }
	return UINT32_MAX; // = MAP_FAILED;
}

template <>
uint32_t syscall_brk<4>(riscv::Machine<4>& machine)
{
	static uint32_t sbrk_end = sbrk_start;
	const uint32_t new_end = machine.sysarg<uint32_t>(0);
	if constexpr (verbose_syscalls) {
		printf("SYSCALL brk called, current = 0x%X new = 0x%X\n", sbrk_end, new_end);
	}
    if (new_end == 0) return sbrk_end;
    sbrk_end = new_end;
    sbrk_end = std::max(sbrk_end, sbrk_start);
    sbrk_end = std::min(sbrk_end, sbrk_max);

	if constexpr (verbose_syscalls) {
		printf("* New sbrk() end: 0x%X\n", sbrk_end);
	}
	return sbrk_end;
}

template <>
uint32_t syscall_stat<4>(riscv::Machine<4>& machine)
{
	const auto  fd      = machine.sysarg<int>(0);
	const auto  buffer  = machine.sysarg<uint32_t>(1);
	if constexpr (verbose_syscalls) {
		printf("SYSCALL stat called, fd = %d  buffer = 0x%X\n",
				fd, buffer);
	}
	if (false) {
		struct stat result;
		std::memset(&result, 0, sizeof(result));
		result.st_dev     = 6;
		result.st_ino     = fd;
		result.st_mode    = 0x21b6;
		result.st_nlink   = 1;
		result.st_rdev    = 265;
		result.st_blksize = 512;
		result.st_blocks  = 0;
		machine.copy_to_guest(buffer, &result, sizeof(result));
		return 0;
	}
	return -EBADF;
}

template <>
uint32_t syscall_spm<4>(riscv::Machine<4>&)
{
    // rt_sigprocmask stubbed
	return 0;
}

template <>
uint32_t syscall_uname<4>(riscv::Machine<4>& machine)
{
	const auto buffer = machine.sysarg<uint32_t>(0);
	if constexpr (verbose_syscalls) {
		printf("SYSCALL uname called, buffer = 0x%X\n", buffer);
	}
    static constexpr int UTSLEN = 65;
    struct uts32 {
        char sysname [UTSLEN];
        char nodename[UTSLEN];
        char release [UTSLEN];
        char version [UTSLEN];
        char machine [UTSLEN];
        char domain  [UTSLEN];
    } uts;
    strcpy(uts.sysname, "RISC-V C++ Emulator");
    strcpy(uts.nodename,"libriscv");
    strcpy(uts.release, "5.0.0");
    strcpy(uts.version, "");
    strcpy(uts.machine, "rv32imac");
    strcpy(uts.domain,  "(none)");

    machine.copy_to_guest(buffer, &uts, sizeof(uts32));
	return 0;
}
