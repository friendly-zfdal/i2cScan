#include "i2c.h"
#include <cstring>
#include <dirent.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

vector<string>
get_i2c_buses ()
{
  vector<string> i2c_buses;
  DIR *dir = opendir ("/sys/bus/i2c/devices");
  if (dir == nullptr)
    {
      std::cerr << "Failed to open /sys/bus/i2c/devices directory"
                << std::endl;
      return i2c_buses;
    }

  dirent *entry;
  while ((entry = readdir (dir)) != nullptr)
    {
      // Check if the entry is a directory, starts with "i2c-" and name's
      // length no longer than 8 symbols
      if (entry->d_type == DT_LNK && strncmp (entry->d_name, "i2c-", 4) == 0
          && strlen (entry->d_name) < 8)
        {
          i2c_buses.push_back ("/dev/" + (string)entry->d_name);
        }
    }
  closedir (dir);
  return i2c_buses;
}

void
print_buf_symbols (unsigned char *buffer, int start)
{
  cout << "    ";
  for (int i = start; i < start + 16; i++)
    {
      if (buffer[i] == 0x00 || buffer[i] == 0xff)
        cout << '.';
      else if (buffer[i] < 32 || buffer[i] >= 127)
        cout << '?';
      else
        cout << buffer[i];
    }
  cout << endl;
}

void
i2c_device_dump (unsigned char *buffer, int device_addr, string bus)
{

  // table init
  cout << "\nI2C Bus:\t" << bus << "  |   I2C-ADDR:\t"
       << "0x" << hex << setw (2) << setfill ('0') << device_addr << endl;
  printf ("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f"
          "    0123456789abcdef\n");
  cout << std::hex << std::setw (2) << std::setfill ('0') << 0 << ":";

  // dump print
  for (int i = 0; i < 256; i++)
    {
      // printing last 16 symbols in hex mode
      cout << " " << std::hex << std::setw (2) << std::setfill ('0')
           << (int)buffer[i];
      if ((i + 1) % 16 == 0)
        {
          // printing last 16 symbols in symbol mode
          print_buf_symbols (buffer, i - 15);
          if (i != 255)
            cout << std::hex << std::setw (2) << std::setfill ('0') << i + 1
                 << ":";
        }
    }
}

int
main ()
{
  int bus;
  // i2c device init
  I2CDevice device;
  memset (&device, 0, sizeof (device));
  device.iaddr_bytes = 1;
  device.page_bytes = 16;
  // buffer for reading data
  unsigned char buffer[256];
  ssize_t size = sizeof (buffer);
  memset (buffer, 0, sizeof (buffer));
  // init list of discovered devices
  std::stringstream discovered_devices;
  discovered_devices << "Discovered I2C devices:\n";

  // getting present in the system i2c buses
  std::vector<string> i2c_buses = get_i2c_buses ();
  vector<string>::iterator it = i2c_buses.begin ();
  int rows_count = 0x00;
  // Note: program dosn't check the bus with "i2c_open" function, due to fact
  // that some buses don't open, but devices which are connected to them could
  // be available
  for (it; it != i2c_buses.end (); it++)
    {
      cout << "\nNow scanning bus" << *it << endl;
      bus = i2c_open ((*it).c_str ());
      device.bus = bus;
      for (int i = 0x00; i < 0x80; i++)
        {
          device.addr = i;
          // check the connection to the device with address = i (from 0 to 127)
          if ((i2c_read (&device, 0x0, buffer, size)) != size)
            {
              continue;
            }
          // if the connection was established successfully - make a dump
          else
            {
              i2c_device_dump (buffer, i, *it);
              // gathering devices
              discovered_devices << "I2C Bus: " << *it << "\t I2C-ADDR: 0x"
                                 << std::hex << std::setw (2)
                                 << std::setfill ('0') << i << endl;
              // clear the buffer after reading one device
              memset (buffer, 0, sizeof (buffer));
            }
        }
      i2c_close (bus);
    }
  // print list of discovered and available I2C devices
  cout << endl << discovered_devices.str ();
}
