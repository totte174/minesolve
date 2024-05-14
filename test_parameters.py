import numpy as np, subprocess
if __name__ == "__main__":
    param_values = np.arange(0, 1.0, 0.05)
    NUM_GAMES = 100_000
    param = "eta"
    for param_v in param_values:
        wins = int(subprocess.run(['./bin/mssolve', f'--test={NUM_GAMES}', f'--{param}="{param_v}"'], capture_output=True).stdout.decode())
        p_hat = wins / NUM_GAMES
        print(param_v, p_hat)