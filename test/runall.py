import re
import subprocess

with open("checks.c") as f:
    string = f.read()

matches = re.findall(r"test_.*_\d", string)

dico = {
    "test_blocking_q_take": 1,
    "test_blocking_q_init": 1,
    "test_blocking_q_destroy": 1,
    "test_blocking_q_put": 1,
    "test_blocking_q_drain": 1,
    "test_blocking_q_drain_at_least": 1,
    "test_blocking_q_peek": 1,
    "test_processor_init": 1,
    "test_processor_destroy": 1,
}

perfect_score = {key: 0 for (key, _) in dico.items()}
points = perfect_score.copy()

failed = 0
for test_case in matches:
    ret = subprocess.call(f"./tests_build/check_tests {test_case}", shell=True)

    perfect_score[test_case[:-2]] += 1

    if ret == 0:
        points[test_case[:-2]] += 1

total = 0
for key, val in dico.items():
    print(f"{key}: {(points[key] / perfect_score[key])}")
    total += (points[key] / perfect_score[key]) * val