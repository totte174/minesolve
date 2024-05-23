import numpy as np, subprocess
if __name__ == "__main__":
    param_values = np.arange(1.0, 2.0, 0.1)
    NUM_GAMES = 100_000
    param = "beta"
    for param_v in param_values:
        wins = int(subprocess.run(['./bin/mssolve', '--eta=0.55', f'--{param}={param_v:.6f}', f'--test={NUM_GAMES}'], capture_output=True).stdout.decode())
        p_hat = wins / NUM_GAMES
        print(f'{param_v:.6f}, {p_hat}')