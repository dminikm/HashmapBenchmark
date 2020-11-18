interface BenchmarkRun {
    time: number,
    hash: number,
}

export interface BenchmarkResult {
    runs: BenchmarkRun[],
    implementation: string,
    correct: boolean,
    num_runs: number,
    num_threads: number,
    total_time: number,
    min_time: number,
    avg_time: number,
    mean_time: number,
    max_time: number,
}

type DeepPartialArr<T extends any[]> =
    (DeepPartial<T[keyof T]>)[];

type DeepPartial<T> =
    T extends any[] ?
        DeepPartialArr<T> :
    T extends {} ?
        { [K in keyof T]?: DeepPartial<T[K]> } :
        T;

export type BenchmarkResultJSON = DeepPartial<BenchmarkResult>;