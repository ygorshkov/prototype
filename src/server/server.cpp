#include <iostream>

#include <server/asio/service.h>
#include <system/cpu.h>

#include <OptionParser.h>

#include "server.h"

using namespace CppCommon;

int main(int argc, char** argv)
{
  auto parser = optparse::OptionParser().version("1.0.0.0");

  parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(1111).help("Server port. Default: %default");
  parser.add_option("-t", "--threads").dest("threads").action("store").type("int").set_default(CPU::PhysicalCores()).help("Count of working threads. Default: %default");

  optparse::Values options = parser.parse_args(argc, argv);

  // Print help
  if (options.get("help"))
  {
      parser.print_help();
      return 0;
  }

  // Server port
  int port = options.get("port");
  int threads = options.get("threads");

  std::cout << "Server port: " << port << std::endl;
  std::cout << "Working threads: " << threads << std::endl;

  std::cout << std::endl;

  // Create a new Asio service
  auto service = std::make_shared<CppServer::Asio::Service>(threads);

  // Start the Asio service
  std::cout << "Asio service starting...";
  service->Start();
  std::cout << "Done!" << std::endl;

  // Create a new server
  auto server = std::make_shared<prototype::Server>(service, port);
  // server->SetupNoDelay(true);
  server->SetupReuseAddress(true);
  server->SetupReusePort(true);

  // Start the server
  std::cout << "Server starting...";
  server->Start();
  std::cout << "Done!" << std::endl;

  std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

  // Perform text input
  std::string line;
  while (getline(std::cin, line))
  {
    if (line.empty())
        break;

    // Restart the server
    if (line == "!")
    {
      std::cout << "Server restarting...";
      server->Restart();
      std::cout << "Done!" << std::endl;
      continue;
    }
  }

  // Stop the server
  std::cout << "Server stopping...";
  server->Stop();
  std::cout << "Done!" << std::endl;

  // Stop the Asio service
  std::cout << "Asio service stopping...";
  service->Stop();
  std::cout << "Done!" << std::endl;

  return 0;
}
