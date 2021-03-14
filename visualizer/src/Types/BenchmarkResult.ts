interface BenchmarkRun {
    value: number,
    hash: number,
}

export interface BenchmarkResult {
    runs: BenchmarkRun[],
    implementation: string,
    correct: boolean,
    num_runs: number,
    num_threads: number,
    total_value: number,
    min_value: number,
    avg_value: number,
    mean_value: number,
    max_value: number,
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