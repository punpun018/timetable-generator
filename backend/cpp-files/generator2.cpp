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
#define vvi vector<vector<int>>
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

void optimizeTT()
{
    for (int i = 0; i < days; i++)
    {
        for (int s = 0; s < 3; s++)
        {
            int ct = 50;
            while (ct-- && TT[i][s].strength > limit)
            {
                for (auto it : TT[i][s].Subjects)
                {
                    for (int j = 0; j < 3; j++)
                        if (j != s)
                        {
                            bool canPlace = true;
                            for (auto itr : TT[i][j].Subjects)
                                if (adjMatrix[it][itr])
                                    canPlace = false;
                            if (canPlace)
                            {
                                TT[i][s].Subjects.erase(it);
                                TT[i][j].Subjects.insert(it);
                                TT[i][s].strength -= subjectData[it].strength;
                                TT[i][j].strength += subjectData[it].strength;
                                goto nextIter;
                            }
                        }
                }
            nextIter:;
            }
        }
    }

    set<int> clashes;
    for (int i = 0; i < days; i++)
        for (int j = 0; j < 3; j++)
            for (int k = j + 1; k < 3; k++)
                for (auto it : TT[i][j].Subjects)
                    for (auto itr : TT[i][k].Subjects)
                        if (adjMatrix[it][itr])
                        {
                            clashes.insert(it);
                            clashes.insert(itr);
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
            for (int j = 0; j < 3; j++)
            {
                if (TT[i][j].Subjects.find(it) != TT[i][j].Subjects.end())

                {
                    TT[i][j].Subjects.erase(it);
                    TT[i][j].strength -= subjectData[it].strength;
                    goto removed;
                }
            }
        }
    removed:;

        int minClashes = INT_MAX, cx = 0, cy = 0;
        for (int i = 0; i < days; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                bool canPlace = true;
                for (auto itr : TT[i][j].Subjects)
                    if (adjMatrix[it][itr])
                        canPlace = false;
                if (canPlace)
                {
                    int tempClashes = 0;
                    for (int k = 0; k < 3; k++)
                        if (k != j)
                            for (auto itr : TT[i][k].Subjects)
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
        sort(TT[i].begin(), TT[i].end(),
             [](const Partition &a, const Partition &b)
             {
                 return a.strength > b.strength;
             });

    for (int i = 0; i < days; i++)
    {
        for (int s = 0; s < 3; s++)
        {
            int ct = 50;
            while (ct-- && TT[i][s].strength > limit)
            {
                for (auto it : TT[i][s].Subjects)
                {
                    for (int j = 0; j < 3; j++)
                        if (j != s)
                        {
                            bool canPlace = true;
                            for (auto itr : TT[i][j].Subjects)
                                if (adjMatrix[it][itr])
                                    canPlace = false;
                            if (canPlace)
                            {
                                TT[i][s].Subjects.erase(it);
                                TT[i][j].Subjects.insert(it);
                                TT[i][s].strength -= subjectData[it].strength;
                                TT[i][j].strength += subjectData[it].strength;
                                goto nextIter2;
                            }
                        }
                }
            nextIter2:;
            }
        }
    }
}

vector<tuple<int, int, int>> minimizeSlotClashes3(const vvi &confMatrix, const vector<vvi> &confMatrix2)
{
    int n = confMatrix.size();
    if (n % 3 != 0)
    {
        throw invalid_argument("Number of slots must be divisible by 3.");
    }

    const int INF = 1e9;
    int maxMask = 1 << n;
    vi dp(maxMask, INF);
    vector<int> choice(maxMask, -1);
    dp[0] = 0;

    for (int mask = 0; mask < maxMask; ++mask)
    {
        int cnt = __builtin_popcount(mask);
        if (cnt % 3 != 0)
            continue;

        int i;
        for (i = 0; i < n; ++i)
            if (mask & (1 << i))
                break;

        for (int j = i + 1; j < n; ++j)
        {
            if (!(mask & (1 << j)))
                continue;
            for (int k = j + 1; k < n; ++k)
            {
                if (!(mask & (1 << k)))
                    continue;

                int newMask = mask ^ (1 << i) ^ (1 << j) ^ (1 << k);

                int cost = 500 * confMatrix2[i][j][k] + confMatrix[i][j] + confMatrix[i][k] + confMatrix[j][k] + dp[newMask];

                if (cost < dp[mask])
                {
                    dp[mask] = cost;
                    choice[mask] = (i << 20) | (j << 10) | k;
                }
            }
        }
    }

    vector<tuple<int, int, int>> result;
    int mask = maxMask - 1;
    while (mask)
    {
        int packed = choice[mask];
        int i = (packed >> 20) & 1023;
        int j = (packed >> 10) & 1023;
        int k = packed & 1023;

        result.emplace_back(i, j, k);

        mask ^= (1 << i);
        mask ^= (1 << j);
        mask ^= (1 << k);
    }

    return result;
}

vector<vvi> buildConfMatrix3(const vector<Partition> &Slots, const vector<Student> &studentData)
{
    int n = Slots.size();
    vector<vector<vector<int>>> confMatrix2(n, vector<vector<int>>(n, vector<int>(n, 0)));

    for (const auto &student : studentData)
    {
        vector<int> enrolledSlots;
        for (int subj : student.subjectsEnrolled)
        {
            for (int s = 0; s < n; s++)
            {
                if (Slots[s].Subjects.count(subj))
                {
                    enrolledSlots.push_back(s);
                }
            }
        }

        int m = enrolledSlots.size();
        for (int a = 0; a < m; a++)
        {
            for (int b = a + 1; b < m; b++)
            {
                for (int c = b + 1; c < m; c++)
                {
                    int i = enrolledSlots[a];
                    int j = enrolledSlots[b];
                    int k = enrolledSlots[c];
                    confMatrix2[i][j][k]++;
                    confMatrix2[i][k][j]++;
                    confMatrix2[j][i][k]++;
                    confMatrix2[j][k][i]++;
                    confMatrix2[k][i][j]++;
                    confMatrix2[k][j][i]++;
                }
            }
        }
    }

    return confMatrix2;
}

void makeTT3()
{
    int n = totalSlots;
    vector<vi> confMatrix(n, vi(n, 0));
    for (int i = 0; i < n; i++)
    {
        for (int j = i; j < n; j++)
        {
            int clashes = 0;
            for (auto it : Slots[i].Subjects)
                for (auto itr : Slots[j].Subjects)
                    clashes += adjMatrix[it][itr];
            confMatrix[i][j] = confMatrix[j][i] = clashes;
        }
    }

    auto confMatrix2 = buildConfMatrix3(Slots, studentData);
    auto triplets = minimizeSlotClashes3(confMatrix, confMatrix2);

    TT.clear();
    TT.resize(days, vector<Partition>(examPerDay));

    for (int d = 0; d < days; d++)
    {
        auto [a, b, c] = triplets[d];
        for (auto it : Slots[a].Subjects)
        {
            TT[d][0].Subjects.insert(it);
            TT[d][0].strength += subjectData[it].strength;
        }
        for (auto it : Slots[b].Subjects)
        {
            TT[d][1].Subjects.insert(it);
            TT[d][1].strength += subjectData[it].strength;
        }
        for (auto it : Slots[c].Subjects)
        {
            TT[d][2].Subjects.insert(it);
            TT[d][2].strength += subjectData[it].strength;
        }
        sort(TT[d].begin(), TT[d].end(),
             [](const Partition &p1, const Partition &p2)
             {
                 return p1.strength > p2.strength;
             });
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

vi minimizeDayClashes(const vector<vi> &confMatrix)
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
            for (int sl = 0; sl < 3; sl++)
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
                for (int sl = 0; sl < 3; sl++)
                {
                    for (auto it : TT[d2][sl].Subjects)
                    {
                        if (studentData[i].subjectsEnrolled.find(it) != studentData[i].subjectsEnrolled.end())

                        {
                            exam2++;
                            break;
                        }
                    }
                }

                int total = exams + exam2;
                if (total == 3)
                {
                    confMatrix[d1][d2] += 1;
                    confMatrix[d2][d1] += 1;
                }
                if (total == 4)
                {
                    confMatrix[d1][d2] += 50;
                    confMatrix[d2][d1] += 50;
                }
                if (total == 5)
                {
                    confMatrix[d1][d2] += 1000;
                    confMatrix[d2][d1] += 1000;
                }
                if (total == 6)
                {
                    confMatrix[d1][d2] += 10000;
                    confMatrix[d2][d1] += 10000;
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
    makeTT3();
    optimizeTT();
    optimizeTT();
    finalOptimization();
    nameJson();
    cout << outJson.dump(2);
    return 0;
}