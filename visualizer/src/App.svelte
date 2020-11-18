<script lang="ts">
    import FileDnD from './Components/FileDnD.svelte';
    import Visualizer from './Components/Visualizer.svelte';
    import type { BenchmarkResult, BenchmarkResultJSON } from './Types/BenchmarkResult';
    import type { FileContent } from './Types/FileContent';

    $: loaded = data.length !== 0;
    let data: BenchmarkResult[] = [];

    function loadData(files: FileContent[]) {
        let newData: BenchmarkResult[] = [];

        for (const file of files) {
            var json = JSON.parse(file.content) as BenchmarkResultJSON;

            // TODO: Do json verification here
            newData = [...newData, json as BenchmarkResult];
        }

        data = newData;
    }
</script>

<style>
    :global(*) {
        padding: 0px;
        margin: 0px;

        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        font-size: 14px;
    }
    
    :global(html, body), main {
        width: 100vw;
        max-width: 100%;

        height: 100vh;

        padding: 0px;
        margin: 0px;
    }

    nav {
        width: 100%;
        height: 45px;

        box-shadow: 0px 2px 4px lightgray;
    }

    h3 {
        font-size: 20px;

        margin: 0px;
        line-height: 45px;

        padding-left: 5px;
    }
</style>

<svelte:head>
    <script src="https://kit.fontawesome.com/361a34c661.js" crossorigin="anonymous"></script>
</svelte:head>

<main>
    <nav>
        <h3>HashmapBenchmark Visualizer</h3>
    </nav>

    {#if !loaded}
        <FileDnD
            onDrop={loadData}
        />
    {:else}
        <Visualizer data={data} />
    {/if}
</main>