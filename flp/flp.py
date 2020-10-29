import flipper
import serial
import click

DEFAULT_TTY = "/dev/ttyUSB0"


@click.group()
def cli():
    pass


@cli.group()
def fs():
    pass


@fs.command()
@click.argument("file_in", type=click.Path(exists=True))
@click.argument("file_out")
@click.option("--port", default=DEFAULT_TTY)
def push(file_in, file_out, port):
    with serial.Serial(port, 115200, timeout=10) as p:
        flipper.file.push(p, file_in, file_out)
    print("OK.")


@fs.command()
@click.argument("file_in")
@click.argument("file_out", type=click.Path())
@click.option("--port", default=DEFAULT_TTY)
def pull(file_in, file_out, port):
    with serial.Serial(port, 115200, timeout=10) as p:
        flipper.file.pull(p, file_in, file_out)
    print("OK.")


if __name__ == "__main__":
    cli()
