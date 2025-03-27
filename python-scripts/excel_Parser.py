import sys
import json
import pandas as pd
from sortedcontainers import SortedDict, SortedSet

def parse_excel(file_path):
    df = pd.read_excel(file_path, engine="openpyxl")

    courses = SortedDict()
    students = SortedDict()
    reversed_students = SortedDict()

    for column in df.columns:
        courses[column] = -1
        for i, student in enumerate(df[column].dropna()):
            students[student] = -1

    for i, column in enumerate(courses):
        courses[column] = i

    for i, student in enumerate(students):
        students[student] = i
        reversed_students[i] = student

    course_size = [0] * len(courses)
    for column in df.columns:
        course_size[courses[column]] = len(df[column].dropna())

    courses_student = [SortedSet() for _ in range(len(courses))]
    for col in df.columns:
        col_index = courses[col]
        for student in df[col].dropna():
            courses_student[col_index].add(students[student])

    student_courses = [SortedSet() for _ in range(len(students))]
    for col in df.columns:
        col_index = courses[col]
        for student in df[col].dropna():
            student_courses[students[student]].add(col_index)

    graph_edges = SortedSet()
    for i in range(len(courses)):
        for student in courses_student[i]:
            for j in range(i + 1, len(courses)):
                if student in courses_student[j]:
                    if i > j:
                        graph_edges.add((j, i))
                    else:
                        graph_edges.add((i, j))

    result = {
        "num_courses": len(courses),
        "num_edges": len(graph_edges),
        "edges": list(graph_edges),
        "courses": {column: {"id": i, "size": course_size[i]} for column, i in courses.items()},
        "students": {student: {"id": i, "courses": list(student_courses[i])} for student, i in students.items()}
    }

    return result

if __name__ == "__main__":
    input_file = sys.stdin.buffer.read()
    temp_file = "temp.xlsx"

    with open(temp_file, "wb") as f:
        f.write(input_file)

    result = parse_excel(temp_file)
    
    print(json.dumps(result))
