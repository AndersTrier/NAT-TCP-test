use async_std::net::{TcpListener, TcpStream};
use futures::stream::StreamExt;
use std::{str, error};
use std::time::Duration;
use async_std::task;
use async_std::prelude::*;
use rlimit::*;

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
    Ok(())
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


#[async_std::main]
async fn main() {
    raise_rlimit_nofile().unwrap_or_else(|error| {
        println!("Falied to update RLIMIT_NOFILE: {:?}. Continuing.", error);
    });

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
