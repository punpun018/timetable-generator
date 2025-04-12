#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
#define vi vector<int>
#define vb vector<bool>
#define vp vector<pair<int, int>>
#define all(x) x.begin(), x.end()

struct Subject
{
    int hash, strength;
    string Name;
    set<int> studentsEnrolled;
};

struct Student
{
    string rollNumber;
    int hash, noOfSubjects;
    set<int> subjectsEnrolled;
};

struct Partition
{
    int strength = 0;
    set<int> Subjects;
    set<int> Students;
};

int subjects, edges, students, days, examPerDay, limit, totalSlots;
vector<set<int>> Graph;
vector<Subject> subjectData;
vector<Student> studentData;
vector<Partition> Slots;
vector<vector<vi>> timeTable;
vector<vi> dp;

json outJson;

void Input(json &jsonData)
{
    subjects = jsonData["adjacencyGraph"]["numberOfCourses"];
    edges = jsonData["adjacencyGraph"]["numberOfEdges"];
    students = jsonData["adjacencyGraph"]["students"].size();
    days = jsonData["adjacencyGraph"]["numberOfDays"];
    examPerDay = jsonData["adjacencyGraph"]["numberOfSlots"];
    limit = jsonData["adjacencyGraph"]["maxStrengthPerSlot"];

    string s;
    int id, str, u, v, wt;
    Graph.resize(subjects);
    subjectData.resize(subjects);
    studentData.resize(students);
    dp.resize(subjects, vi(subjects));

    for (auto it : jsonData["adjacencyGraph"]["edges"])
    {
        u = it[0], v = it[1], wt = it[2];
        Graph[u].insert(v);
        Graph[v].insert(u);
        dp[u][v] = dp[v][u] = wt;
    }

    int i = 0;
    for (auto &[course_name, course_details] : jsonData["adjacencyGraph"]["courses"].items())
    {
        s = course_name;
        id = course_details["id"];
        if (id != i)
            cout << "Fail";
        str = course_details["size"];
        subjectData[i].Name = s;
        subjectData[i].hash = id;
        subjectData[i++].strength = str;
    }

    i = 0;
    for (auto &[x, y] : jsonData["adjacencyGraph"]["students"].items())
    {
        s = x;
        studentData[i].rollNumber = s;
        id = y["id"];
        studentData[i].hash = id;
        str = y["courses"].size();
        studentData[i].noOfSubjects = str;
        for (int it : y["courses"])
            studentData[i].subjectsEnrolled.insert(it);
        i++;
    }

    totalSlots = days * examPerDay;
}

void nameJson()
{
    vector<vector<vector<string>>> v;
    v.resize(days, vector<vector<string>>(examPerDay));

    for (int i = 0; i < days; i++)
        for (int j = 0; j < examPerDay; j++)
            for (auto it : timeTable[i][j])
                v[i][j].push_back(subjectData[it].Name + " (" + to_string(subjectData[it].strength) + ")");

    outJson["days"] = days;
    outJson["slotsPerDay"] = examPerDay;
    outJson["examTT"] = timeTable;
    outJson["subjNames"] = v;
}

void makeTempTT()
{
    timeTable.resize(days, vector<vi>(examPerDay));
    int extra = totalSlots - Slots.size();
    if (extra <= -1)
    {
        return;
    }

    for (int i = 0; i < extra; i++)
    {
        timeTable[i / examPerDay][i % examPerDay].push_back(*Slots[0].Subjects.begin());
        Slots[0].Subjects.erase(Slots[0].Subjects.begin());
    }

    for (int i = extra, s = 0; i < totalSlots; i++, s++)
    {
        for (auto it : Slots[s].Subjects)
            timeTable[i / examPerDay][s % examPerDay].push_back(it);
    }
    nameJson();
}

void dsaturColoring()
{
    int n = subjects;
    vector<set<int>> neighborColors(n);
    vi saturation(n, 0), degree(n, 0), color(n, -1);

    for (int i = 0; i < n; ++i)
        degree[i] = Graph[i].size();

    vector<bool> used(n, false);
    int colored = 0;

    while (colored < n)
    {
        int u = -1;
        int maxSat = -1, maxDeg = -1;

        for (int i = 0; i < n; ++i)
        {
            if (!used[i])
            {
                if (saturation[i] > maxSat || (saturation[i] == maxSat && degree[i] > maxDeg))
                {
                    maxSat = saturation[i];
                    maxDeg = degree[i];
                    u = i;
                }
            }
        }

        int c = 0;
        while (neighborColors[u].count(c))
            ++c;
        color[u] = c;
        used[u] = true;
        ++colored;

        for (int v : Graph[u])
        {
            if (!used[v])
            {
                if (!neighborColors[v].count(c))
                {
                    neighborColors[v].insert(c);
                    ++saturation[v];
                }
            }
        }
    }

    int totalSlots = *max_element(all(color)) + 1;
    Slots.resize(totalSlots);
    for (int i = 0; i < subjects; i++)
    {
        Slots[color[i]].strength += subjectData[i].strength;
        Slots[color[i]].Subjects.insert(i);
    }

    std::sort(Slots.begin(), Slots.end(), [](const Partition &A, const Partition &B)
              { return A.Subjects.size() > B.Subjects.size(); });

    for (int i = 0; i < totalSlots; i++)
    {
        cerr << "Slot : " << i + 1 << " Strength : " << Slots[i].strength << "\n";
        for (auto it : Slots[i].Subjects)
            cerr << subjectData[it].Name << " ";
        cerr << "\n";
    }
}

void Print()
{
    int count = 0;
    for (auto it : timeTable)
    {
        count++;
        cout << "Day -: " << count << "\n";
        cout << "Slot 1 : ";
        for (auto itr : it[0])
            cout << subjectData[itr].Name << " ";
        cout << "\nSlot 2 : ";
        for (auto itr : it[1])
            cout << subjectData[itr].Name << " ";
        cout << "\n";
    }
}

int main()
{
    json jsonData;
    cin >> jsonData;
    Input(jsonData);
    dsaturColoring();
    makeTempTT();
    cout << outJson.dump(2) << endl;
    return 0;
}