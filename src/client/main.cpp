#include <server/asio/service.h>
#include <benchmark/reporter_console.h>
#include <system/cpu.h>
#include <threads/thread.h>
#include <time/timestamp.h>

#include <OptionParser.h>

#include "client.h"
#include "message/factory.h"

int main(int argc, const char* argv[])
{
  auto parser = optparse::OptionParser().version("1.0.0.0");
  
  parser.add_option("-a", "--address").dest("address").set_default("127.0.0.1").help("Server address. Default: %default");
  parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(1111).help("Server port. Default: %default");
  parser.add_option("-t", "--threads").dest("threads").action("store").type("int").set_default(CppCommon::CPU::PhysicalCores()).help("Count of working threads. Default: %default");
  parser.add_option("-z", "--seconds").dest("seconds").action("store").type("int").set_default(10).help("Count of seconds to benchmarking. Default: %default");
  parser.add_option("-v", "--verbose").dest("verbose").action("store").type("bool").set_default(false).help("Verbose. Default: %default");

  optparse::Values options = parser.parse_args(argc, argv);
  
  // Print help
  if (options.get("help"))
  {
    parser.print_help();
    return 0;
  }
  
  // Client parameters
  std::string address(options.get("address"));
  int port = options.get("port");
  int threads_count = options.get("threads");
  int seconds_count = options.get("seconds");
  bool verbose = options.get("verbose");
  
  if (verbose) {
    std::cout << "string size: " << sizeof(std::string) << std::endl;
    std::cout << "Pong size: " << sizeof(prototype::message::Pong) << std::endl;
    std::cout << "Request size: " << sizeof(prototype::message::Request) << std::endl;
    std::cout << "Reply size: " << sizeof(prototype::message::Reply) << std::endl;

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << 1 << std::endl;
    std::cout << "Seconds to benchmarking: " << seconds_count << std::endl;
    std::cout << "Verbose: " << verbose << std::endl;

    std::cout << std::endl;
  }
  // Create a new Asio service
  auto service = std::make_shared<CppServer::Asio::Service>(threads_count);
  // Start the Asio service
  std::cout << "Asio service starting...";
  service->Start();
  std::cout << "Done!" << std::endl;
  
  // Create echo clients
  auto client = std::make_shared<prototype::Client<prototype::message::SequentialFactory>>(service, address, port, verbose);
  // client->SetupNoDelay(true);
  
  // Connect clients
  std::cout << "Clients connecting...";
  client->ConnectAsync();
  std::cout << "Done!" << std::endl;
  while (!client->IsConnected())
    CppCommon::Thread::Yield();
  std::cout << "All clients connected!" << std::endl;
  
  // Wait for benchmarking
  std::cout << "Benchmarking...";
  CppCommon::Thread::Sleep(seconds_count * 1000);
  std::cout << "Done!" << std::endl;
  
  // Disconnect clients
  std::cout << "Clients disconnecting...";
  client->DisconnectAsync();
  std::cout << "Done!" << std::endl;
  while (client->IsConnected())
    CppCommon::Thread::Yield();
  std::cout << "All clients disconnected!" << std::endl;
  
  // Stop the Asio service
  std::cout << "Asio service stopping...";
  service->Stop();
  std::cout << "Done!" << std::endl;
  std::cout << std::endl;
  
  std::cout << "Errors: " << total_errors << std::endl;
  std::cout << std::endl;
  
  auto print_measures = [] (const prototype::Measurer& measurer) {
    std::cout << measurer.name << " count: " << measurer.count() << '\n';
    std::cout << measurer.name << " avg: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(measurer.avg_ns()) << '\n';
    std::cout << measurer.name << " total: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(measurer.total_ns())<< '\n';
  };

  print_measures(client->ping_pong);
  print_measures(client->ping_pong_ser);
  print_measures(client->request_reply);
  print_measures(client->request_reply_ser);
  
  return 0;
}
