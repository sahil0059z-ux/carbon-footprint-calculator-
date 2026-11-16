/*
 * ============================================================================
 *  PROJECT : Personal Carbon Footprint Calculator
 *  SDG     : SDG 13 - Climate Action (also supports SDG 7, 11 and 12)
 *  LANGUAGE: C++ (Object Oriented Programming) - compiles with C++11 or later
 *
 *  OOP concepts demonstrated
 *  -------------------------
 *  1. Abstraction       : abstract base class  Activity  (pure virtual functions)
 *  2. Inheritance       : Transport / Electricity / Cooking / Food / Waste
 *                         Activity classes derive from Activity
 *  3. Polymorphism      : virtual calculateEmission(), getDetails(), getTip()
 *  4. Encapsulation     : private/protected data with public interfaces
 *  5. Operator overload : operator<<, operator>, operator+
 *  6. Templates         : readNumber<T>() generic validated input
 *  7. Exception handling: custom InvalidInputException
 *  8. STL               : vector, map, pair, shared_ptr
 *  9. File handling     : saves report (.txt) and history (.csv), reads history
 *
 *  NOTE: Emission factors are approximate values used for educational purposes
 *  (based on commonly published figures for India). All results are monthly
 *  emissions in kg of CO2-equivalent.
 * ============================================================================
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <limits>
#include <ctime>
using namespace std;

// ---------------------------------------------------------------------------
//  Constants (approximate emission factors)
// ---------------------------------------------------------------------------
const double GRID_FACTOR_KG_PER_KWH = 0.82;   // India grid electricity, kg CO2 / kWh
const double LPG_KG_PER_CYLINDER    = 14.2;   // domestic LPG cylinder weight (kg)
const double LPG_FACTOR_KG_PER_KG   = 2.98;   // kg CO2 per kg of LPG burnt
const double WASTE_FACTOR_KG_PER_KG = 0.45;   // kg CO2 per kg of un-recycled waste
const double WEEKS_PER_MONTH        = 4.33;
const double DAYS_PER_MONTH         = 30.0;
const double KG_CO2_PER_TREE_YEAR   = 21.0;   // CO2 absorbed by one tree in a year
const double INDIA_AVG_TONNES_YEAR  = 2.0;    // approx. per-capita, India
const double WORLD_AVG_TONNES_YEAR  = 4.7;    // approx. per-capita, world

// ---------------------------------------------------------------------------
//  Custom exception
// ---------------------------------------------------------------------------
class InvalidInputException : public runtime_error {
public:
    explicit InvalidInputException(const string& msg) : runtime_error(msg) {}
};

// ---------------------------------------------------------------------------
//  Template: validated numeric input
// ---------------------------------------------------------------------------
template <typename T>
T readNumber(const string& prompt, T minVal, T maxVal) {
    while (true) {
        cout << prompt;
        T value;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "  Please enter a value between " << minVal
                 << " and " << maxVal << ".\n";
        } else {
            if (cin.eof()) throw runtime_error("Input stream closed.");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Invalid input. Please enter a number.\n";
        }
    }
}

// ===========================================================================
//  ABSTRACT BASE CLASS : Activity
// ===========================================================================
class Activity {
protected:
    string name;                       // e.g. "Transport (Bus)"
public:
    explicit Activity(const string& n) : name(n) {}
    virtual ~Activity() {}

    // Pure virtual functions -> every activity MUST define these
    virtual double calculateEmission() const = 0;   // kg CO2 per month
    virtual string getCategory() const = 0;
    virtual string getDetails() const = 0;
    virtual string getTip() const = 0;

    string getName() const { return name; }

    // Operator overloading: compare two activities by emission
    bool operator>(const Activity& other) const {
        return calculateEmission() > other.calculateEmission();
    }

    friend ostream& operator<<(ostream& os, const Activity& a) {
        os << left << setw(26) << a.getName()
           << right << setw(10) << fixed << setprecision(2)
           << a.calculateEmission() << " kg CO2   [" << a.getDetails() << "]";
        return os;
    }
};

// Operator overloading: running total  (double + Activity)
double operator+(double sum, const Activity& a) {
    return sum + a.calculateEmission();
}

// ===========================================================================
//  DERIVED CLASS 1 : TransportActivity
// ===========================================================================
class TransportActivity : public Activity {
private:
    int    mode;
    double kmPerMonth;

    static const vector<pair<string, double> >& modes() {
        static vector<pair<string, double> > m;
        if (m.empty()) {
            m.push_back(make_pair("Petrol Car",        0.192));
            m.push_back(make_pair("Diesel Car",        0.171));
            m.push_back(make_pair("Motorbike/Scooter", 0.103));
            m.push_back(make_pair("City Bus",          0.089));
            m.push_back(make_pair("Metro/Train",       0.041));
            m.push_back(make_pair("Domestic Flight",   0.255));
            m.push_back(make_pair("Cycle/Walk",        0.000));
        }
        return m;
    }
public:
    TransportActivity(int modeChoice, double km) : Activity("Transport"), mode(modeChoice), kmPerMonth(km) {
        if (modeChoice < 1 || modeChoice > (int)modes().size())
            throw InvalidInputException("Invalid transport mode.");
        if (km < 0)
            throw InvalidInputException("Distance cannot be negative.");
        name = "Transport (" + modes()[mode - 1].first + ")";
    }
    static void showModes() {
        for (size_t i = 0; i < modes().size(); i++)
            cout << "   " << (i + 1) << ". " << modes()[i].first << "\n";
    }
    static int modeCount() { return (int)modes().size(); }

    double calculateEmission() const { return kmPerMonth * modes()[mode - 1].second; }
    string getCategory() const { return "Transport"; }
    string getDetails() const {
        ostringstream s; s << fixed << setprecision(0) << kmPerMonth << " km/month";
        return s.str();
    }
    string getTip() const {
        if (mode <= 2) return "Try carpooling, or switch some trips to metro/bus to cut transport emissions.";
        if (mode == 6) return "Replace short flights with train travel where possible.";
        return "Great choice! Public/active transport keeps your footprint low.";
    }
};

// ===========================================================================
//  DERIVED CLASS 2 : ElectricityActivity
// ===========================================================================
class ElectricityActivity : public Activity {
private:
    double units;          // kWh per month
    double solarPercent;   // share of electricity from solar (0-100)
public:
    ElectricityActivity(double kWh, double solar) : Activity("Electricity"), units(kWh), solarPercent(solar) {
        if (kWh < 0)                    throw InvalidInputException("Electricity units cannot be negative.");
        if (solar < 0 || solar > 100)   throw InvalidInputException("Solar share must be 0-100%.");
    }
    double calculateEmission() const { return units * (1.0 - solarPercent / 100.0) * GRID_FACTOR_KG_PER_KWH; }
    string getCategory() const { return "Electricity"; }
    string getDetails() const {
        ostringstream s; s << fixed << setprecision(0) << units << " kWh, " << solarPercent << "% solar";
        return s.str();
    }
    string getTip() const {
        if (units > 200) return "Use 5-star appliances, LED lights and consider rooftop solar (SDG 7).";
        return "Your usage is modest. Switch off standby devices to save even more.";
    }
};

// ===========================================================================
//  DERIVED CLASS 3 : CookingActivity (LPG)
// ===========================================================================
class CookingActivity : public Activity {
private:
    double cylindersPerMonth;
public:
    explicit CookingActivity(double cylinders) : Activity("Cooking Fuel (LPG)"), cylindersPerMonth(cylinders) {
        if (cylinders < 0) throw InvalidInputException("LPG cylinders cannot be negative.");
    }
    double calculateEmission() const {
        return cylindersPerMonth * LPG_KG_PER_CYLINDER * LPG_FACTOR_KG_PER_KG;
    }
    string getCategory() const { return "Cooking"; }
    string getDetails() const {
        ostringstream s; s << fixed << setprecision(2) << cylindersPerMonth << " cylinder(s)/month";
        return s.str();
    }
    string getTip() const { return "Use pressure cookers and keep lids on pots to save LPG."; }
};

// ===========================================================================
//  DERIVED CLASS 4 : FoodActivity
// ===========================================================================
class FoodActivity : public Activity {
private:
    int diet;   // 1 = vegetarian, 2 = eggetarian, 3 = non-vegetarian
public:
    explicit FoodActivity(int dietChoice) : Activity("Food / Diet"), diet(dietChoice) {
        if (dietChoice < 1 || dietChoice > 3) throw InvalidInputException("Invalid diet choice.");
    }
    double calculateEmission() const {
        const double perDay[3] = { 1.7, 2.0, 3.3 };   // kg CO2 per day
        return perDay[diet - 1] * DAYS_PER_MONTH;
    }
    string getCategory() const { return "Food"; }
    string getDetails() const {
        const char* names[3] = { "Vegetarian", "Eggetarian", "Non-vegetarian" };
        return names[diet - 1];
    }
    string getTip() const {
        if (diet == 3) return "Having a few plant-based meals each week lowers food emissions noticeably.";
        return "Eating local, seasonal food keeps your food footprint low.";
    }
};

// ===========================================================================
//  DERIVED CLASS 5 : WasteActivity
// ===========================================================================
class WasteActivity : public Activity {
private:
    double kgPerWeek;
    double recyclePercent;
public:
    WasteActivity(double kg, double recycle) : Activity("Household Waste"), kgPerWeek(kg), recyclePercent(recycle) {
        if (kg < 0) throw InvalidInputException("Waste cannot be negative.");
        if (recycle < 0 || recycle > 100) throw InvalidInputException("Recycling share must be 0-100%.");
    }
    double calculateEmission() const {
        return kgPerWeek * WEEKS_PER_MONTH * (1.0 - recyclePercent / 100.0) * WASTE_FACTOR_KG_PER_KG;
    }
    string getCategory() const { return "Waste"; }
    string getDetails() const {
        ostringstream s; s << fixed << setprecision(1) << kgPerWeek << " kg/week, " << setprecision(0) << recyclePercent << "% recycled";
        return s.str();
    }
    string getTip() const {
        if (recyclePercent < 50) return "Segregate waste and compost kitchen scraps to raise your recycling share.";
        return "Good recycling habits! Keep avoiding single-use plastics.";
    }
};

// ===========================================================================
//  CLASS : FootprintTracker  (holds a user's activities - composition)
// ===========================================================================
class FootprintTracker {
private:
    string userName;
    vector<shared_ptr<Activity> > activities;
public:
    explicit FootprintTracker(const string& name) : userName(name) {}

    void addActivity(shared_ptr<Activity> a) { activities.push_back(a); }
    bool isEmpty() const { return activities.empty(); }
    string getUserName() const { return userName; }

    double totalMonthly() const {
        double total = 0.0;
        for (size_t i = 0; i < activities.size(); i++)
            total = total + *activities[i];          // uses overloaded operator+
        return total;
    }
    double totalAnnualTonnes() const { return totalMonthly() * 12.0 / 1000.0; }

    map<string, double> categoryBreakdown() const {
        map<string, double> m;
        for (size_t i = 0; i < activities.size(); i++)
            m[activities[i]->getCategory()] += activities[i]->calculateEmission();
        return m;
    }

    shared_ptr<Activity> highestActivity() const {
        shared_ptr<Activity> top = activities[0];
        for (size_t i = 1; i < activities.size(); i++)
            if (*activities[i] > *top) top = activities[i];   // uses overloaded operator>
        return top;
    }

    string rating() const {
        double t = totalAnnualTonnes();
        if (t <= INDIA_AVG_TONNES_YEAR) return "LOW (at or below the India average)";
        if (t <= WORLD_AVG_TONNES_YEAR) return "MODERATE (above India avg, below world avg)";
        return "HIGH (above the world average)";
    }

    void printReport(ostream& os) const {
        os << "\n==================== CARBON FOOTPRINT REPORT ====================\n";
        os << " Name : " << userName << "\n";
        os << "-----------------------------------------------------------------\n";
        for (size_t i = 0; i < activities.size(); i++)
            os << " " << *activities[i] << "\n";
        os << "-----------------------------------------------------------------\n";
        os << fixed << setprecision(2);
        double total = totalMonthly();
        os << " Total per month  : " << total << " kg CO2\n";
        os << " Total per year   : " << totalAnnualTonnes() << " tonnes CO2\n";
        os << " India average    : ~" << INDIA_AVG_TONNES_YEAR << " tonnes/year | World average: ~"
           << WORLD_AVG_TONNES_YEAR << " tonnes/year\n";
        os << " Rating           : " << rating() << "\n";
        os << "\n Category breakdown:\n";
        map<string, double> m = categoryBreakdown();
        for (map<string, double>::iterator it = m.begin(); it != m.end(); ++it) {
            double pct = total > 0 ? (it->second / total) * 100.0 : 0.0;
            os << "   " << left << setw(12) << it->first << right << setw(10) << it->second
               << " kg  (" << setw(5) << setprecision(1) << pct << "%)\n";
            os << setprecision(2);
        }
        shared_ptr<Activity> top = highestActivity();
        os << "\n Biggest contributor : " << top->getName() << "\n";
        os << " Tips to reduce your footprint (SDG 13):\n";
        for (size_t i = 0; i < activities.size(); i++)
            os << "   - " << activities[i]->getName() << ": " << activities[i]->getTip() << "\n";
        os << fixed << setprecision(0);
        os << "\n To offset one year of emissions you would need about "
           << (totalMonthly() * 12.0 / KG_CO2_PER_TREE_YEAR) << " trees.\n";
        os << "=================================================================\n";
    }

    // ---------------- File handling ----------------
    bool saveReport(const string& fileName) const {
        ofstream out(fileName.c_str());
        if (!out) return false;
        printReport(out);
        return true;
    }
    bool appendHistory(const string& fileName) const {
        ofstream out(fileName.c_str(), ios::app);
        if (!out) return false;
        time_t now = time(0);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&now));
        out << buf << "," << userName << "," << fixed << setprecision(2)
            << totalMonthly() << "," << totalAnnualTonnes() << "\n";
        return true;
    }
    static void showHistory(const string& fileName) {
        ifstream in(fileName.c_str());
        if (!in) { cout << "\n No saved history found yet.\n"; return; }
        cout << "\n----------------------- SAVED HISTORY -----------------------\n";
        cout << left << setw(18) << "Date/Time" << setw(14) << "Name"
             << setw(16) << "kg CO2/month" << "tonnes/year\n";
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            string date, user, monthly, annual;
            getline(ss, date, ',');  getline(ss, user, ',');
            getline(ss, monthly, ','); getline(ss, annual, ',');
            cout << left << setw(18) << date << setw(14) << user
                 << setw(16) << monthly << annual << "\n";
        }
        cout << "-------------------------------------------------------------\n";
    }
};

// ---------------------------------------------------------------------------
//  Menu helper functions
// ---------------------------------------------------------------------------
void printBanner() {
    cout << "=================================================================\n";
    cout << "        PERSONAL CARBON FOOTPRINT CALCULATOR  (SDG 13)\n";
    cout << "             Measure it. Understand it. Reduce it.\n";
    cout << "=================================================================\n";
}

void addTransport(FootprintTracker& t) {
    cout << "\n Select your mode of travel:\n";
    TransportActivity::showModes();
    int mode = readNumber<int>(" Choice: ", 1, TransportActivity::modeCount());
    double km = readNumber<double>(" Distance travelled per month (km): ", 0.0, 100000.0);
    t.addActivity(make_shared<TransportActivity>(mode, km));
    cout << " -> Transport activity added.\n";
}
void addElectricity(FootprintTracker& t) {
    double kwh   = readNumber<double>("\n Electricity used per month (kWh): ", 0.0, 100000.0);
    double solar = readNumber<double>(" Percentage from solar/renewables (0-100): ", 0.0, 100.0);
    t.addActivity(make_shared<ElectricityActivity>(kwh, solar));
    cout << " -> Electricity activity added.\n";
}
void addCooking(FootprintTracker& t) {
    double c = readNumber<double>("\n LPG cylinders used per month (e.g. 0.5): ", 0.0, 50.0);
    t.addActivity(make_shared<CookingActivity>(c));
    cout << " -> Cooking activity added.\n";
}
void addFood(FootprintTracker& t) {
    cout << "\n Select your diet:\n   1. Vegetarian\n   2. Eggetarian\n   3. Non-vegetarian\n";
    int d = readNumber<int>(" Choice: ", 1, 3);
    t.addActivity(make_shared<FoodActivity>(d));
    cout << " -> Food activity added.\n";
}
void addWaste(FootprintTracker& t) {
    double kg  = readNumber<double>("\n Household waste produced per week (kg): ", 0.0, 1000.0);
    double rec = readNumber<double>(" Percentage recycled/composted (0-100): ", 0.0, 100.0);
    t.addActivity(make_shared<WasteActivity>(kg, rec));
    cout << " -> Waste activity added.\n";
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------
int main() {
    const string REPORT_FILE  = "footprint_report.txt";
    const string HISTORY_FILE = "footprint_history.csv";

    printBanner();
    cout << " Enter your name: ";
    string name;
    getline(cin, name);
    if (name.empty()) name = "User";
    FootprintTracker tracker(name);

    try {
        while (true) {
            cout << "\n-------------------------- MENU --------------------------\n";
            cout << " 1. Add Transport        4. Add Food / Diet\n";
            cout << " 2. Add Electricity      5. Add Household Waste\n";
            cout << " 3. Add Cooking (LPG)    6. View Report & Tips\n";
            cout << " 7. Save Report to File  8. View Saved History\n";
            cout << " 9. Exit\n";
            int choice = readNumber<int>(" Enter your choice: ", 1, 9);

            try {
                switch (choice) {
                    case 1: addTransport(tracker);   break;
                    case 2: addElectricity(tracker); break;
                    case 3: addCooking(tracker);     break;
                    case 4: addFood(tracker);        break;
                    case 5: addWaste(tracker);       break;
                    case 6:
                        if (tracker.isEmpty()) cout << "\n Add at least one activity first.\n";
                        else tracker.printReport(cout);
                        break;
                    case 7:
                        if (tracker.isEmpty()) { cout << "\n Add at least one activity first.\n"; break; }
                        if (tracker.saveReport(REPORT_FILE) && tracker.appendHistory(HISTORY_FILE))
                            cout << "\n Report saved to '" << REPORT_FILE
                                 << "' and history updated in '" << HISTORY_FILE << "'.\n";
                        else
                            cout << "\n Error: could not write to file.\n";
                        break;
                    case 8: FootprintTracker::showHistory(HISTORY_FILE); break;
                    case 9:
                        cout << "\n Thank you, " << name << "! Small changes add up to climate action.\n";
                        return 0;
                }
            } catch (const InvalidInputException& e) {
                cout << "\n Error: " << e.what() << "\n";
            }
        }
    } catch (const exception& e) {
        cout << "\n " << e.what() << " Exiting.\n";
    }
    return 0;
}
