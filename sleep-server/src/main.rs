use async_std::net::{TcpListener, TcpStream};
use futures::stream::StreamExt;
use std::{str, error};
use std::time::Duration;
use async_std::task;
use async_std::prelude::*;

async fn handle_client(mut stream: TcpStream) -> Result<(), Box<dyn error::Error>>{
    // Read up to 64 bytes from stream
    let mut buf: [u8; 64] = [0; 64];
    let n = stream.read(&mut buf).await?;

    if n == 0 {
        // Connection was closed
        return Ok(());
    }

    let sleeptimestr = str::from_utf8(&buf[0..n])?.trim();
    let sleeptime: u64 = sleeptimestr.parse()?;
    if sleeptime > 60*60*24 {
        return Err("not sleeping that long".into());
    }
    task::sleep(Duration::from_secs(sleeptime)).await;

    stream.write(&buf[0..n]).await?;
    stream.flush().await?;
    return Ok(());
}

#[async_std::main]
async fn main() {
    let listener = TcpListener::bind("0.0.0.0:27182").await.unwrap();

    listener.incoming().for_each_concurrent(
        /* limit */ None,
        | tcpstream | async move {
            match tcpstream {
                Ok(stream) => {
                    if let Err(e) = handle_client(stream).await {
                        eprintln!("{}", e);
                    }
                }
                Err(e) => {
                    eprintln!("listener.incoming() error: {}", e);
                }
            }
        }
    ).await
}
