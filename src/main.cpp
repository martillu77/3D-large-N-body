// cmake -S . -B build
// cmake --build build

#include "Configuration.h"
#include "Simulation.h"

#include <iostream>
#include <sstream>
#include <string>

void print_help() {
    std::cout
        << "Commands:\n"
        << "  load_config <filename>  Load configuration from file\n"
        << "  start                   Start simulation\n"
        << "  pause                   Pause simulation\n"
        << "  resume                  Resume simulation\n"
        << "  stop                    Stop simulation\n"
        << "  status                  Show current state\n"
        << "  dump                    Show simulation information\n"
        << "  help                    Show this help\n"
        << "  quit                    Exit program\n\n";
}

void end_response() {
    std::cout << "END\n" << std::flush;
}



int main() {
    Simulation simulation;
    Configuration configuration;

    std::cout << "N-body engine 0.1.0\n";
    end_response();

    std::string line;  // Read the queries from the python interface

    while (std::getline(std::cin, line)) {
        
        std::istringstream input(line);

        std::string command;
        input >> command;

        // Read a configuration card (file)
        if (command == "load_config") {   // first word is the command, second word is filename
            std::string filename;
            input >> filename;

            if (filename.empty()) std::cout << "ERROR Missing configuration file\n";

            else if (configuration.load(filename)) {
                simulation.load_configuration(configuration);
                std::cout << "OK\n";
            }
            else std::cout << "ERROR Cannot load configuration: " << filename << '\n';

            end_response();
        }

        else if (command == "start") {
            simulation.start();
            std::cout << "OK\n";
            end_response();
        }
        else if (command == "pause") {
            simulation.pause();
            std::cout << "OK\n";
            end_response();
        }
        else if (command == "resume") {
            simulation.resume();
            std::cout << "OK\n";
            end_response();
        }
        else if (command == "stop") {
            simulation.stop();
            std::cout << "OK\n";
            end_response();
        }
        else if (command == "status") {
            std::cout << simulation.status() << '\n';
            end_response();
        }
        else if (command == "dump") {
            std::cout << simulation.dump();
            end_response();
        }
        else if (command == "help") {
            print_help();
            end_response();
        }
        else if (command == "quit" || command == "exit") {
            std::cout << "OK\n";
            end_response();
            break;
        }
        else if (!command.empty()) {
            std::cout << "ERROR Unknown command: " << command << '\n';
            end_response();
        }
    }

    simulation.stop();

    return 0;
}