# carbon-footprint-calculator-
🌍 Personal Carbon Footprint Calculator

Measure it. Understand it. Reduce it.

A C++ Object-Oriented Programming project that calculates an individual's monthly and annual carbon footprint based on everyday lifestyle activities.

The project helps users understand their environmental impact and provides personalized suggestions to reduce carbon emissions.

🎯 Project Objectives

- Calculate monthly and annual carbon emissions.
- Identify major sources of personal emissions.
- Promote sustainable lifestyle choices.
- Encourage awareness about climate change.
- Support United Nations Sustainable Development Goal 13 (Climate Action).

✨ Key Features

- 🚗 Transport Emissions: Calculates emissions from cars, bikes, buses, metro, flights and cycling/walking.
- ⚡ Electricity Consumption: Tracks electricity usage and renewable energy contribution.
- 🔥 Cooking Fuel: Calculates emissions from domestic LPG consumption.
- 🥗 Food & Diet: Estimates emissions based on vegetarian, eggetarian and non-vegetarian diets.
- ♻️ Household Waste: Calculates waste emissions based on waste generation and recycling percentage.
- 📊 Carbon Footprint Report: Displays monthly and annual emissions with category-wise breakdown.
- 💡 Personalized Suggestions: Provides tips to reduce environmental impact.
- 💾 File Handling: Saves reports in TXT format and historical records in CSV format.
- 🌳 Tree Offset Estimate: Estimates the number of trees required to offset annual emissions.

🛠️ Technologies Used

- C++
- Object-Oriented Programming
- Standard Template Library (STL)
- File Handling
- Exception Handling
- Templates
- Operator Overloading

🧠 OOP Concepts Implemented

Concept| Implementation
Abstraction| Abstract Activity base class
Inheritance| Five derived activity classes
Polymorphism| Virtual functions
Encapsulation| Private and protected data
Operator Overloading| "+", ">" and "<<"
Templates| Generic validated input function
Exception Handling| Custom InvalidInputException
STL| Vector, Map, Pair and Shared Pointer

📂 Project Structure

Carbon-Footprint-Calculator/
CarbonFootprintCalculator.cpp
 README.md
gitignore
 Generated Files/
 footprint_report.txt
 footprint_history.csv

The TXT and CSV files are generated when the program runs and saves a report.

▶️ How to Run

Step 1: Clone the repository

git clone YOUR_REPOSITORY_URL

Step 2: Navigate to the project folder

cd Carbon-Footprint-Calculator

Step 3: Compile the program

g++ -std=c++11 CarbonFootprintCalculator.cpp -o calculator

Step 4: Run the program

./calculator

On Windows:

calculator.exe

📈 Emission Categories

The calculator evaluates five major categories:

1. Transportation
2. Electricity
3. Cooking Fuel (LPG)
4. Food & Diet
5. Household Waste

Note: Emission factors are approximate educational estimates, not official real-time measurements. Actual emissions may vary depending on location, energy sources and lifestyle.

🌱 Sustainable Development Goals

This project primarily supports:

SDG 13 – Climate Action

It also relates to:

- SDG 7 – Affordable and Clean Energy
- SDG 11 – Sustainable Cities and Communities
- SDG 12 – Responsible Consumption and Production

🚀 Future Improvements

- Develop a graphical user interface (GUI).
- Build a web-based version of the calculator.
- Add visual charts and analytics.
- Introduce user profiles and saved sessions.
- Improve emission factor accuracy using verified regional datasets.

👨‍💻 Author

Sahil Kumar

B.Tech – Information Technology

Maharaja Agrasen Institute of Technology, New Delhi


⭐ If you find this project interesting, consider giving the repository a star!

Built with C++ | For a Greener Future 🌍
