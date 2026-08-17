from http.server import HTTPServer, BaseHTTPRequestHandler
import subprocess
import os
import urllib.parse

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            self.send_file("index.html", "text/html")
        elif self.path == "/style.css":
            self.send_file("style.css", "text/css")
        elif self.path == "/script.js":
            self.send_file("script.js", "application/javascript")
        else:
            self.send_response(404)
            self.end_headers()

    def send_file(self, filename, content_type):
        with open(filename, "rb") as f:
            data = f.read()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        data = self.rfile.read(length)

        boundary = self.headers.get("Content-Type").split("boundary=")[1].encode()
        parts = data.split(b"--" + boundary)

        filename = "upload.bin"
        filedata = b""

        for part in parts:
            if b"filename=" in part:
                header_end = part.find(b"\r\n\r\n")
                header = part[:header_end].decode(errors="ignore")
                start = header.find('filename="') + 10
                end = header.find('"', start)
                filename = os.path.basename(header[start:end])
                filedata = part[header_end + 4:]
                filedata = filedata.rstrip(b"\r\n-")
                break

        query = urllib.parse.urlparse(self.path)
        params = urllib.parse.parse_qs(query.query)
        operation = params.get("operation", ["compress"])[0]

        input_file = "server_input"
        output_file = "server_output"

        with open(input_file, "wb") as f:
            f.write(filedata)

        if operation == "compress":
            result = subprocess.run(["./huffman", "compress", input_file, output_file])
            output_name = filename + ".huff"
        else:
            result = subprocess.run(["./huffman", "decompress", input_file, output_file])
            output_name = "decompressed_" + filename

        if result.returncode != 0:
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Operation failed")
            return

        with open(output_file, "rb") as f:
            output_data = f.read()

        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Disposition", f'attachment; filename="{output_name}"')
        self.send_header("Content-Length", str(len(output_data)))
        self.end_headers()
        self.wfile.write(output_data)

if __name__ == "__main__":
    print("Server running at http://localhost:8000")
    HTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
