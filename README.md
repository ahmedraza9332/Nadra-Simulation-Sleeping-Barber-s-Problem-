NADRA Simulation - The Sleeping Barber Problem
A C++ simulation of Pakistan's NADRA (National Database and Registration Authority) service system, inspired by the classic "Sleeping Barber Problem" from operating systems theory.
📋 Overview
This project demonstrates a real-world application of the sleeping barber synchronization problem by modeling NADRA's citizen verification services. The simulation uses multithreading to represent the interaction between NADRA workers and citizens seeking various services like CNIC applications, passport renewals, and other documentation services.
🧵 Problem Description
The Classic Sleeping Barber Problem adapted for NADRA:

Workers (Barbers): NADRA service representatives who process citizen requests
Customers (Citizens): People seeking NADRA services
Waiting Area: Limited seating capacity in the NADRA office
Service Counter: Where actual document processing occurs

How It Works

N Workers operate as independent threads, each capable of serving citizens
M Customers arrive as separate threads seeking services
Fixed-size waiting area with limited seating capacity
Customer behavior:

If waiting area has space → customer sits and waits for service
If waiting area is full → customer leaves and returns later


Worker behavior:

If customers are waiting → worker serves the next customer
If no customers → worker sleeps until a customer arrives



🛠️ Technical Implementation

Language: C++
Threading: POSIX threads (pthreads)
Synchronization: Mutexes and condition variables
Platform: Linux only

🚀 Getting Started
Prerequisites

Linux operating system
GCC compiler with pthread support

Compilation
bashg++ nadra_problem.cpp -pthread -o nadra_problem
Running the Simulation
bash./nadra_problem
📊 Simulation Parameters
You can modify the following parameters in the source code:

Number of NADRA workers
Number of citizens
Waiting area capacity
Service time duration

🎯 Learning Objectives
This simulation demonstrates:

Thread synchronization in concurrent programming
Producer-consumer problem variants
Resource management with limited capacity
Real-world application of OS concepts

🇵🇰 About NADRA
NADRA (National Database and Registration Authority) is Pakistan's premier organization responsible for:

Issuing Computerized National Identity Cards (CNIC)
Passport services
Birth/death certificates
Family registration certificates

📝 Author
Ahmed Raza - ahmedraza9332
📄 License
This project is open source and available under the MIT License.

This simulation is created for educational purposes to demonstrate operating system concepts through a familiar, real-world scenario.
