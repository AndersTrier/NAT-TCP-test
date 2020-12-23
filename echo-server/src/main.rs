use std::net::{TcpListener, TcpStream};
use std::thread;
use std::io::Read;
use std::io::Write;

fn handle_client(mut stream: TcpStream) {
    // Read up to 64 bytes from stream
    let mut buf = [0; 64];
    match stream.read(&mut buf) {
        Ok(n) => {
            if n == 0 {
                // Connection was closed
                return;
            }
            let _ = stream.write_all(&buf[0..n]);
        }
        Err(e) => {
            eprintln!("read() error: {}", e);
        }
    }
}

fn main() {
    let listener = TcpListener::bind("0.0.0.0:31415").unwrap();

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                thread::spawn(move || {
                    handle_client(stream);
                });
            }
            Err(e) => {
                eprintln!("listener.incoming() error: {}", e);
            }
        }
    }
}
