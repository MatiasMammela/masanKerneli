#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;

# Define the ISO file
my $iso = "image.iso";
system("make");
# Check if Limine has already been built
unless (-f "limine/limine-bios-cd.bin") {
    system("make -C limine") == 0 or die "Failed to build Limine: $!";
}

# Ensure necessary directories exist
system("mkdir -p iso_root/boot iso_root/boot/limine iso_root/EFI/BOOT") == 0
    or die "Failed to create directories: $!";

# Copy the kernel and Limine files
system("cp -v bin/myos.elf iso_root/boot/") == 0 or die "Failed to copy kernel: $!";
system("cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/") == 0
    or die "Failed to copy Limine files: $!";
system("cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/") == 0 or die "Failed to copy BOOTX64.EFI: $!";
system("cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/") == 0 or die "Failed to copy BOOTIA32.EFI: $!";

my $xorriso_command = "xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin " .
    "-no-emul-boot -boot-load-size 4 -boot-info-table " .
    "--efi-boot boot/limine/limine-uefi-cd.bin " .
    "-efi-boot-part --efi-boot-image --protective-msdos-label " .
    "iso_root -o $iso";

# Run the command and handle failure
system($xorriso_command) == 0 or die "Failed to create ISO: $!";

# Install Limine for legacy BIOS boot (optional if BIOS boot is required)
system("./limine/limine bios-install $iso") == 0 or die "Failed to install Limine: $!";

# QEMU flags
my @qemu_flags = (
    "-boot", "d",                      # Boot from the CD-ROM
    "-cdrom", $iso,                    # Specify the CD-ROM image to use (ISO file)
    "-D", "log.txt",                   # Enable logging and write logs to log.txt
    "-d", "int",                       # Enable debug output for interrupts
    "-M", "smm=off",                   # Disable SMM (System Management Mode)
    "-no-reboot",                      # Do not reboot the VM on shutdown
    "-no-shutdown",                    # Do not power off the VM on shutdown
    "-serial", "file:/dev/stdout",     # Redirect serial output to standard output
    "-monitor", "stdio"                # Use standard input/output for the QEMU monitor
);

# Fork and run QEMU
my $pid = fork();
if ($pid == 0) {
    # Child process: Run QEMU
    exec("qemu-system-x86_64", @qemu_flags);
    exit(0);
} 

$SIG{'INT'} = sub {
    # Handle Ctrl+C (SIGINT)
    print "Killing QEMU...\n";
    kill 'TERM', $pid;  # Send terminate signal to QEMU
    waitpid($pid, 0);   # Wait for QEMU to exit
    exit(0);
};


# Parent process continues, waiting for QEMU
waitpid($pid, 0);