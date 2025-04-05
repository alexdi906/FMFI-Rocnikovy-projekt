import time
import os
import subprocess


try:
    subprocess.run(["make"], check=True)
except subprocess.CalledProcessError as e:
    print("Error during make: ", e.stderr)

def run(files, line_graph, run_directory = True):
    print(files)
    for algo in ["sat", "backtracking"]:
        start_time = time.time()
        for graph_file in os.listdir(files) if run_directory else files:
            try:
                res = subprocess.run(["./main.out", f"{files}/{graph_file}" if
                                         run_directory else graph_file, "-a", algo, f"--linegraph={line_graph}"], check=True, capture_output=True, text=True)
            except subprocess.CalledProcessError as e2:
                print("Error during execution: ", e2.stderr)

        end_time = time.time()
        print(algo + " " + str(round(end_time - start_time, 3)))


# run(["graphs/4regular/11_4_3.g6"],"false", False)
# run("graphs/4regular/chromatic_index_4", "false")
# files = []
# for i in range(5, 13):
#     files.append(f"graphs/4regular/{i:02}_4_3.g6")
# print(files)
# run(files, "false", False)
# run("graphs/3regular/chromatic_index_3", "true")
# run(["../no_ECD/17_4_3.clawfree.g6.C_NO"], "false", False)
# run(["../no_ECD/13_4_3.3c.g6.C_NO"], "false",False)
# run(["../no_ECD/16_4_3.clawfree.g6.C_NO", "../no_ECD/15_4_3.clawfree.g6.C_NO", "../no_ECD/14_4_3.clawfree.g6.C_NO","../no_ECD/13_4_3.clawfree.g6.C_NO" ], "false", False)
run(["graphs/3regular/14_3_3.g6"], "true", False)
