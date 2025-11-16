#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <climits>
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
vector<vector<set<int>>> timeTable;
vector<vi> adjMatrix;
vector<vector<Partition>> TT;

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
    adjMatrix.resize(subjects, vi(subjects));

    for (auto it : jsonData["adjacencyGraph"]["edges"])
    {
        u = it[0], v = it[1], wt = it[2];
        Graph[u].insert(v);
        Graph[v].insert(u);
        adjMatrix[u][v] = adjMatrix[v][u] = wt;
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
    timeTable.resize(days, vector<set<int>>(examPerDay));

    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (auto it : TT[i][j].Subjects)
                timeTable[i][j].insert(it);
        }
    }

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

vector<pair<int, int>> minimizeSlotClashes(const vector<vi> &confMatrix)
{
    int n = confMatrix.size();
    if (n % 2 != 0)
    {
        throw invalid_argument("Number of slots must be even.");
    }

    const int INF = 1e9;
    int maxMask = 1 << n;
    vi dp(maxMask, INF);
    vi choice(maxMask, -1);
    dp[0] = 0;

    for (int mask = 0; mask < maxMask; ++mask)
    {
        int cnt = __builtin_popcount(mask);
        if (cnt % 2 != 0)
            continue;
        int i;
        for (i = 0; i < n; ++i)
            if (mask & (1 << i))
                break;

        for (int j = i + 1; j < n; ++j)
        {
            if (mask & (1 << j))
            {
                int newMask = mask ^ (1 << i) ^ (1 << j);
                int cost = confMatrix[i][j] + dp[newMask];
                if (cost < dp[mask])
                {
                    dp[mask] = cost;
                    choice[mask] = (i << 8) | j;
                }
            }
        }
    }

    vector<pair<int, int>> result;
    int mask = maxMask - 1;
    while (mask)
    {
        int pair = choice[mask];
        int i = pair >> 8;
        int j = pair & 255;
        result.emplace_back(i, j);
        mask ^= (1 << i);
        mask ^= (1 << j);
    }

    return result;
}

void optimizeTT()
{
    for (int i = 0; i < days; i++)
    {
        int ct = 50;
        int sub = 0;
        while (ct-- && TT[i][0].strength > limit)
        {
            for (auto it : TT[i][0].Subjects)
            {
                bool canPlace = true;
                for (auto itr : TT[i][1].Subjects)
                    if (adjMatrix[it][itr])
                        canPlace = false;

                if (canPlace)
                {
                    TT[i][0].Subjects.erase(it);
                    TT[i][1].Subjects.insert(it);
                    TT[i][0].strength -= subjectData[it].strength;
                    TT[i][1].strength += subjectData[it].strength;
                    break;
                }
            }
        }
    }

    set<int> clashes;

    for (int i = 0; i < days; i++)
    {
        for (auto it : TT[i][0].Subjects)
            for (auto itr : TT[i][1].Subjects)
                if (adjMatrix[it][itr])
                {
                    clashes.insert(it);
                    clashes.insert(itr);
                }
    }

    vector<int> vec;
    for (auto it : clashes)
        vec.push_back(it);
    for (auto it : clashes)
        vec.push_back(it);

    for (auto it : vec)
    {
        for (int i = 0; i < days; i++)
        {
            for (int j = 0; j < examPerDay; j++)
            {
                if (std::find(TT[i][j].Subjects.begin(), TT[i][j].Subjects.end(), it) != TT[i][j].Subjects.end())
                {
                    TT[i][j].Subjects.erase(it);
                    TT[i][j].strength -= subjectData[it].strength;
                    break;
                }
            }
        }

        int minClashes = INT_MAX, cx, cy;
        for (int i = 0; i < days; i++)
        {
            for (int j = 0; j < examPerDay; j++)
            {
                bool canPlace = true;
                for (auto itr : TT[i][j].Subjects)
                    if (adjMatrix[it][itr])
                        canPlace = false;

                if (canPlace)
                {
                    int tempClashes = 0;
                    int oI = (j == 0) ? 1 : 0;
                    for (auto itr : TT[i][oI].Subjects)
                        tempClashes += adjMatrix[it][itr];
                    if (tempClashes < minClashes)
                        minClashes = tempClashes, cx = i, cy = j;
                }
            }
        }

        TT[cx][cy].Subjects.insert(it);
        TT[cx][cy].strength += subjectData[it].strength;
    }

    for (int i = 0; i < days; i++)
    {
        if (TT[i][1].strength > TT[i][0].strength)
        {
            Partition temp;
            temp = TT[i][1];
            TT[i][1] = TT[i][0];
            TT[i][0] = temp;
        }
    }

    for (int i = 0; i < days; i++)
    {
        int ct = 50;
        int sub = 0;
        while (ct-- && TT[i][0].strength > limit)
        {
            for (auto it : TT[i][0].Subjects)
            {
                bool canPlace = true;
                for (auto itr : TT[i][1].Subjects)
                    if (adjMatrix[it][itr])
                        canPlace = false;

                if (canPlace)
                {
                    TT[i][0].Subjects.erase(it);
                    TT[i][1].Subjects.insert(it);
                    TT[i][0].strength -= subjectData[it].strength;
                    TT[i][1].strength += subjectData[it].strength;
                    break;
                }
            }
        }
    }
}

void makeTT()
{
    vector<vi> confMatrix(totalSlots, vi(totalSlots));

    for (int i = 0; i < totalSlots; i++)
    {
        for (int j = i; j < totalSlots; j++)
        {
            int clashes = 0;
            for (auto it : Slots[i].Subjects)
                for (auto itr : Slots[j].Subjects)
                    clashes += adjMatrix[it][itr];
            confMatrix[i][j] = confMatrix[j][i] = clashes;
        }
    }

    auto pairs = minimizeSlotClashes(confMatrix);
    TT.resize(days, vector<Partition>(examPerDay));

    for (int i = 0; i < days; i++)
    {
        for (auto it : Slots[pairs[i].first].Subjects)
        {
            TT[i][0].Subjects.insert(it);
            TT[i][0].strength += subjectData[it].strength;
        }
        for (auto it : Slots[pairs[i].second].Subjects)
        {
            TT[i][1].Subjects.insert(it);
            TT[i][1].strength += subjectData[it].strength;
        }

        if (TT[i][1].strength > TT[i][0].strength)
        {
            Partition temp;
            temp = TT[i][1];
            TT[i][1] = TT[i][0];
            TT[i][0] = temp;
        }
    }
}

void genrateSlots()
{
    int NUM_SLOTS = days * examPerDay;
    int n = adjMatrix.size();
    vector<vi> slots(NUM_SLOTS);
    vi slotWt(NUM_SLOTS, 0);
    vi subjSlot(n, -1);

    vi degree(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            degree[i] += adjMatrix[i][j];

    vi subjects(n);
    iota(subjects.begin(), subjects.end(), 0);
    sort(subjects.begin(), subjects.end(), [&](int a, int b)
         { return degree[a] > degree[b]; });

    for (int u : subjects)
    {
        int bestSlot = -1;
        int minInc = INT_MAX;

        for (int s = 0; s < NUM_SLOTS; ++s)
        {
            int intraClash = 0;

            for (int v : slots[s])
            {
                intraClash += adjMatrix[u][v];
            }

            if (intraClash < minInc)
            {
                minInc = intraClash;
                bestSlot = s;
            }
        }

        slots[bestSlot].push_back(u);
        subjSlot[u] = bestSlot;
    }

    Slots.resize(slots.size());
    for (int i = 0; i < slots.size(); i++)
        for (auto it : slots[i])
        {
            Slots[i].Subjects.insert(it);
            Slots[i].strength += subjectData[it].strength;
        }
}

void Print()
{
    int count = 0;
    for (auto it : TT)
    {
        count++;
        cout << "Day -: " << count << "\n";
        cout << "Slot 1 : " << it[0].strength << " ";
        for (auto itr : it[0].Subjects)
            cout << subjectData[itr].Name << " ";
        cout << "\nSlot 2 : " << it[1].strength << " ";
        for (auto itr : it[1].Subjects)
            cout << subjectData[itr].Name << " ";
        cout << "\n";
    }
}

vi minimizeDayClashes(vector<vi> &confMatrix)
{
    int n = confMatrix.size();
    vector<bool> used(n, false);
    vi order;

    int curr = 0;
    order.push_back(curr);
    used[curr] = true;

    for (int step = 1; step < n; ++step)
    {
        int nextDay = -1, minClash = INT_MAX;
        for (int j = 0; j < n; ++j)
        {
            if (!used[j] && confMatrix[curr][j] < minClash)
            {
                minClash = confMatrix[curr][j];
                nextDay = j;
            }
        }
        if (nextDay != -1)
        {
            order.push_back(nextDay);
            used[nextDay] = true;
            curr = nextDay;
        }
    }

    auto calcClash = [&](const vi &ord)
    {
        int sum = 0;
        for (int i = 0; i + 1 < n; ++i)
            sum += confMatrix[ord[i]][ord[i + 1]];
        return sum;
    };

    int bestClash = calcClash(order);
    bool improved = true;

    while (improved)
    {
        improved = false;
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                swap(order[i], order[j]);
                int newClash = calcClash(order);
                if (newClash < bestClash)
                {
                    bestClash = newClash;
                    improved = true;
                }
                else
                {
                    swap(order[i], order[j]);
                }
            }
        }
    }
    return order;
}

void finalOptimization()
{
    vector<vi> confMatrix(days, vi(days));
    for (int i = 0; i < students; i++)
    {
        for (int d1 = 0; d1 < days; d1++)
        {
            int exams = 0;
            for (int sl = 0; sl < examPerDay; sl++)
            {
                for (auto it : TT[d1][sl].Subjects)
                {
                    if (studentData[i].subjectsEnrolled.find(it) != studentData[i].subjectsEnrolled.end())
                    {
                        exams++;
                        break;
                    }
                }
            }

            for (int d2 = d1 + 1; d2 < days; d2++)
            {
                int exam2 = 0;
                for (int sl = 0; sl < examPerDay; sl++)
                {
                    for (auto it : TT[d2][sl].Subjects)
                    {
                        if (std::find(studentData[i].subjectsEnrolled.begin(), studentData[i].subjectsEnrolled.end(), it) != studentData[i].subjectsEnrolled.end())
                        {
                            exam2++;
                            break;
                        }
                    }
                }

                if (exams + exam2 == 3)
                {
                    confMatrix[d1][d2]++;
                    confMatrix[d2][d1]++;
                }
                if (exams + exam2 == 4)
                {
                    confMatrix[d1][d2] += 2000;
                    confMatrix[d2][d1] += 2000;
                }
            }
        }
    }

    vi order = minimizeDayClashes(confMatrix);
    vector<vector<Partition>> temp = TT;
    for (int i = 0; i < days; i++)
        temp[i] = TT[order[i]];

    TT = temp;
}

int main()
{
    json jsonData;
    cin >> jsonData;
    Input(jsonData);
    outJson = jsonData;
    genrateSlots();
    makeTT();
    optimizeTT();
    optimizeTT();
    finalOptimization();
    nameJson();
    cout << outJson.dump(2);
    return 0;
}