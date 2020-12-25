use async_std::net::TcpListener;
use async_std::net::TcpStream;
use futures::stream::StreamExt;
//use std::time::Duration;
//use async_std::task;

use async_std::prelude::*;

async fn handle_client(mut stream: TcpStream) {
    //task::sleep(Duration::from_secs(5)).await;

    // Read up to 64 bytes from stream
    let mut buf: [u8; 64] = [0; 64];
    match stream.read(&mut buf).await {
        Ok(n) => {
            if n == 0 {
                // Connection was closed
                return;
            }
            let _ = stream.write(&buf[0..n]).await;
            let _ = stream.flush().await;
        }
        Err(e) => {
            eprintln!("read() error: {}", e);
        }
    }
}

#[async_std::main]
async fn main() {
    let listener = TcpListener::bind("0.0.0.0:31415").await.unwrap();

    listener.incoming().for_each_concurrent(
        /* limit */ None,
        | tcpstream | async move {
            match tcpstream {
                Ok(stream) => {
                    handle_client(stream).await;
                }
                Err(e) => {
                    eprintln!("listener.incoming() error: {}", e);
                }
            }
        }
    ).await
}
