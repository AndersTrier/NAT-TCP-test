use std::net::{TcpListener, TcpStream};
use std::thread;
use std::io::Read;
use std::io::Write;
use rlimit::*;

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

fn raise_rlimit_nofile() -> std::io::Result<()> {
    // Allow this process to have many open connections (file descriptors)
    // Check GETRLIMIT(2) - RLIMIT_NOFILE for more info
    let (nofile_soft, nofile_hard) = getrlimit(Resource::NOFILE)?;
    if nofile_soft != nofile_hard {
        // GETRLIMIT(2):
        // The hard limit acts  as  a  ceiling  for  the  soft
        // limit:  an  unprivileged process may set only its soft limit to a value
        // in the range from 0 up to the hard limit
        return setrlimit(Resource::NOFILE, nofile_hard, nofile_hard);
    }
    Ok(())
}

fn main() {
    raise_rlimit_nofile().unwrap_or_else(|error| {
        println!("Falied to update RLIMIT_NOFILE: {:?}. Continuing.", error);
    });

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
