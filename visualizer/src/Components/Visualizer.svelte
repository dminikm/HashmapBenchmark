<script lang="ts">
    import type { BenchmarkResult } from '../Types/BenchmarkResult';
import Graph from './Graph.svelte';
    export let data: BenchmarkResult[] = [];

    function getAllowedImpls(data: BenchmarkResult[]) {
        var set = new Set(data.map((x) => x.implementation));

        var impls: [string, boolean][] = [];
        for (const impl of set) {
            impls.push([impl, true]);
        }

        return impls;
    }

    $: allowedImpls = getAllowedImpls(data);
    $: console.log(allowedImpls);

    $: filteredData = data.filter((x) => allowedImpls.filter((y) => y[0] === x.implementation && y[1] === true).length !== 0);
    $: console.log(filteredData);
</script>

<style>
    .container {
        display: flex;
        flex-direction: column;

        width: 100%;
        height: 100%;

        align-items: center;
    }

    .checkbox-container {
        display: flex;
        flex-direction: row;
    }

    .checkbox-container > * {
        margin: 5px;
    }
</style>

<div class="container">
    <div style="width: 80%; height: 60%;margin-top: 35px;">
        <Graph data={filteredData} />
    </div>

    <div class="checkbox-container">
        {#each allowedImpls as [impl, val], i}
            <label for="{`check_${i}`}">
                <input id={`check_${i}`} type="checkbox" checked={val} on:change={() => { allowedImpls[i] = [impl, !val]; } }> {impl}
            </label>
        {/each}
    </div>
</div>