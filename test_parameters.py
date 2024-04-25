import numpy as np
import subprocess, math

file = open("alpha.csv", "a")
alphas = np.arange(0, 0.05, 0.0001)
NUM_GAMES = 100

for alpha in alphas:
    wins = int(subprocess.run(['./bin/mssolve', f'--test={NUM_GAMES}', f'--alpha="{alpha}"'], capture_output=True).stdout.decode())
    p_hat = wins / NUM_GAMES
    lower_confidence = p_hat - 1.65 * math.sqrt(p_hat * (1-p_hat) / NUM_GAMES)
    print(alpha, p_hat, lower_confidence)