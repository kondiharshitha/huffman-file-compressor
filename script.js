async function processFile(operation) {
    const input = document.getElementById("fileInput");
    const status = document.getElementById("status");

    if (!input.files.length) {
        status.textContent = "Please select a file first.";
        return;
    }

    const file = input.files[0];
    const formData = new FormData();
    formData.append("file", file);

    status.textContent = operation === "compress" ? "Compressing..." : "Decompressing...";

    try {
        const response = await fetch(`/process?operation=${operation}`, {
            method: "POST",
            body: formData
        });

        if (!response.ok) {
            throw new Error("Operation failed");
        }

        const blob = await response.blob();
        const url = window.URL.createObjectURL(blob);
        const link = document.createElement("a");

        link.href = url;
        link.download = operation === "compress" ? file.name + ".huff" : "decompressed_" + file.name;
        document.body.appendChild(link);
        link.click();
        link.remove();

        window.URL.revokeObjectURL(url);
        status.textContent = "Done! Your file has been downloaded.";
    } catch (error) {
        status.textContent = "Something went wrong.";
    }
}
