<script lang="ts">
    import type { FileContent } from "../Types/FileContent";

    async function handleFileLoading(e: DragEvent) {
        let files: FileContent[] = [];
        
        for (const file of e.dataTransfer.files) {
            const content = await file.text();
            const name = file.name;

            files = [...files, {name, content}]
        }

        onDrop(files);
    }

    export let onDrop: (files: FileContent[]) => void;
</script>

<style>
    .dnd-container {
        position: relative;

        width: 100%;
        height: 100%;
    }

    input[type="file"] {
        width: 100%;
        height: 100%;

        opacity: 0;
    }

    .dnd-help-container {
        position: absolute;
        display: flex;

        flex-direction: column;
        align-items: center;
        justify-content: center;

        left: 50%;
        top: 50%;

        transform: translate(-50%, -50%);

        pointer-events: none;
    }
</style>

<div class="dnd-container">
    <input type="file" multiple on:drop={handleFileLoading}>
    <div class="dnd-help-container">
        <i class="fas fa-file-import fa-4x"></i>
        <strong>Add benchmark results by dragging it here!</strong>
    </div>
</div>