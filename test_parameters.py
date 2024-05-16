import numpy as np, subprocess
if __name__ == "__main__":
    param_values = np.arange(0.8, 0.85, 0.05)
    NUM_GAMES = 1_000_000
    param = "eta"
    for param_v in param_values:
        print(['./bin/mssolve', f'--{param}={param_v:.2f}', f'--test={NUM_GAMES}'])
        wins = int(subprocess.run(['./bin/mssolve', f'--{param}={param_v:.2f}', f'--test={NUM_GAMES}'], capture_output=True).stdout.decode())
        p_hat = wins / NUM_GAMES
        print(param_v, p_hat)