<script lang="ts">
    export let tipX = 0;
    export let tipY = 0;
    export let text = "";

    var cvs = document.createElement('canvas');
    var ctx = cvs.getContext('2d');

    $: textWidth = ctx.measureText(text).width * 2;
    let textHeight = 14;

    ctx.font = `${textHeight} 'Segoe UI'`;
</script>

<polyline
    style="stroke: #000000; fill: #FFFFFF; stroke-width: 1;"
    points={`
        ${tipX},${tipY}
        ${tipX + 5},${tipY - 10}
        ${tipX + (textWidth / 2)},${tipY - 10}
        ${tipX + (textWidth / 2)},${tipY - 10 - (textHeight * 2)}
        ${tipX - (textWidth / 2)},${tipY - 10 - (textHeight * 2)}
        ${tipX - (textWidth / 2)},${tipY - 10}
        ${tipX - 5},${tipY - 10}
        ${tipX},${tipY}
    `}
/>

<text
    x={tipX}
    y={tipY - 10 - textHeight}
    alignment-baseline="middle"
    text-anchor="middle"
>{text}</text>