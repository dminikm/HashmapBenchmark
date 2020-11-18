<script lang="ts">
    import type { each } from "svelte/internal";
import type { BenchmarkResult } from "../Types/BenchmarkResult";
    export let data: BenchmarkResult[] = [];
    
    $: processedData = processData(data);
    
    type ProcessedData = Map<string, Map<number, BenchmarkResult>>;
    function processData(data: BenchmarkResult[]): ProcessedData {
        let processedData: ProcessedData = new Map();
        
        for (const result of data) {
            if (!processedData.has(result.implementation)) {
                processedData.set(result.implementation, new Map());
            }

            let impl = processedData.get(result.implementation);

            if (!impl.has(result.num_threads)) {
                impl.set(result.num_threads, result);
            }
        }

        return processedData;
    }

    function getRanges(minTime: number, maxTime: number, availHeight: number): number[] {
        const numRanges = Math.floor(Math.max(availHeight / 100, 1));

        let ranges = [];
        for (let i = 0; i < numRanges; i++) {
            ranges = [...ranges, Math.floor(minTime + (((maxTime - minTime) / (numRanges - 1)) * i))];
        }

        return ranges;
    }

    function getThreads(data: ProcessedData): number[] {
        const arr = Array.from(data).map(([x, y]) => Array.from(y).map(([x, z]) => z.num_threads));

        if (arr.length > 0) {
            return arr[0].sort((a, b) => a - b);
        }

        return [];
    }

    function getCenterYForBench(bench: BenchmarkResult): number {
        let biggerThan = 0;
        let smallerThan = 0;
        
        for (let i = 0; i < ranges.length; i++) {
            const range = ranges[i];

            if (bench.mean_time > range) {
                biggerThan = i;
                smallerThan = i + 1;
            } else {
                break;
            }
        }

        const bigger = ranges[biggerThan];
        const smaller = ranges[smallerThan] || bigger;

        const perc = (bench.mean_time - bigger) / (smaller - bigger);
        return (biggerThan * ySpacing) + (ySpacing * perc);
    }

    $: numThreads = Math.max(...Array.from(processedData).map(([key, value]) => value.size), 1);
    $: maxTime = Math.max(...Array.from(processedData).map(([key, value]) => Math.max(...Array.from(value).map(([key, x]) => x.max_time))));
    $: minTime = Math.min(...Array.from(processedData).map(([key, value]) => Math.min(...Array.from(value).map(([key, x]) => x.min_time))));

    let graphHeight: number;
    let graphWidth: number;

    const leftBarWidth = 100;
    const rightBarWidth = 200;
    const bottomBarHeight = 40;

    $: availHeight = graphHeight - bottomBarHeight;
    $: availWidth = (graphWidth - leftBarWidth) - rightBarWidth;

    $: ranges = [
        ...getRanges(minTime - 1000000, maxTime + 1000000, availHeight),
    ];

    $: threads = getThreads(processedData);

    $: ySpacing = availHeight / ranges.length;
    $: xSpacing = availWidth / numThreads;

    const colors = [
        "#E21836",
        "#F47E55",
        "#87C440",
        "#3792CB",
        "#CDC884",
        "#FFE800",
    ];

    let fontSize = 12;
</script>

<style>
    div, div > svg {
        width: 100%;
        height: 100%;
    }
</style>

<div bind:clientWidth={graphWidth} bind:clientHeight={graphHeight}>
    <svg>
        {#each ranges as range, i }
            <text x={0} y={graphHeight - bottomBarHeight - (i * ySpacing)}>{Math.floor(range / 1000000)}ms</text>
            {#if i > 0}
                <line
                    x1={leftBarWidth}
                    y1={graphHeight - bottomBarHeight - (fontSize / 2) - (i * ySpacing)}
                    x2={leftBarWidth + availWidth}
                    y2={graphHeight - bottomBarHeight - (fontSize / 2) - (i * ySpacing)}
                    style="stroke:#d3d3d3;stroke-width:1;"
                />
            {:else}
                <line
                    x1={leftBarWidth}
                    y1={graphHeight - bottomBarHeight - (fontSize / 2) - (i * ySpacing)}
                    x2={leftBarWidth + availWidth}
                    y2={graphHeight - bottomBarHeight - (fontSize / 2) - (i * ySpacing)}
                    style="stroke:#000000;stroke-width:2;"
                />
            {/if}
        {/each}

        {#each threads as thread, i}
            <text x={leftBarWidth + (xSpacing * i) + (xSpacing / 2)} y={graphHeight - (fontSize / 2) - (bottomBarHeight / 2)} text-anchor="middle">{thread}</text>
        {/each}

        {#each Array.from(processedData) as [impl, threadMap], color_index}
            <rect
                x={leftBarWidth + availWidth + 10}
                y={((availHeight / 2) - ((processedData.size / 2) * ((fontSize / 2) + 20))) + ((color_index + 1) * ((fontSize / 2) + 20)) - 6}
                width={6}
                height={6}
                style={`fill: ${colors[color_index]};`}
            />
            <text
                x={leftBarWidth + availWidth + (rightBarWidth / 2)}
                y={((availHeight / 2) - ((processedData.size / 2) * ((fontSize / 2) + 20))) + ((color_index + 1) * ((fontSize / 2) + 20))}
                text-anchor="middle"
            >{impl}</text>
        {/each}

        {#each Array.from(processedData) as [impl, threadMap], color_index}
            {#each Array.from(threadMap).sort(([k, a], [k2, b]) => a.num_threads - b.num_threads) as [num_threads, benchmark], i}
                {#if i > 0}
                    <line
                        x1={leftBarWidth + (xSpacing * (i - 1)) + (xSpacing / 2)}
                        y1={graphHeight - (fontSize / 2) - bottomBarHeight - getCenterYForBench(Array.from(threadMap).sort(([k, a], [k2, b]) => a.num_threads - b.num_threads)[i - 1][1])}

                        x2={leftBarWidth + (xSpacing * i) + (xSpacing / 2)}
                        y2={graphHeight - (fontSize / 2) - bottomBarHeight - getCenterYForBench(benchmark)}

                        style={`stroke: ${colors[color_index]}; stroke-width: 2;`}
                    />
                {/if}
            {/each}
        {/each}
    </svg>
</div>