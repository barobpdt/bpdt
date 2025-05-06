import socket
import ipaddress
import concurrent.futures
from typing import Tuple, List
import argparse

def get_hostname(ip: str) -> str:
    try:
        hostname = socket.gethostbyaddr(ip)[0]
        return hostname
    except (socket.herror, socket.gaierror):
        return "Unknown"

def check_port(ip: str, port: int, timeout: float = 1.0) -> bool:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        result = sock.connect_ex((ip, port))
        return result == 0
    except:
        return False
    finally:
        sock.close()

def scan_ip(ip: str, ports: List[int]) -> List[Tuple[str, int, str]]:
    results = []
    for port in ports:
        if check_port(ip, port):
            hostname = get_hostname(ip)
            results.append((ip, port, hostname))
    return results

def main():
    parser = argparse.ArgumentParser(description='Subnet Port Scanner')
    parser.add_argument('subnet', help='Subnet to scan (e.g., 192.168.1.0/24)')
    parser.add_argument('--ports', '-p', nargs='+', type=int, default=[80, 443, 22, 21],
                       help='Ports to scan (default: 80, 443, 22, 21)')
    parser.add_argument('--timeout', '-t', type=float, default=1.0,
                       help='Connection timeout in seconds (default: 1.0)')
    args = parser.parse_args()

    try:
        network = ipaddress.ip_network(args.subnet)
        print(f"Scanning subnet: {args.subnet}")
        print(f"Checking ports: {args.ports}")
        print("-" * 50)

        with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
            future_to_ip = {
                executor.submit(scan_ip, str(ip), args.ports): str(ip)
                for ip in network.hosts()
            }

            for future in concurrent.futures.as_completed(future_to_ip):
                ip = future_to_ip[future]
                try:
                    results = future.result()
                    for ip, port, hostname in results:
                        print(f"IP: {ip:<15} Port: {port:<5} Hostname: {hostname}")
                except Exception as e:
                    print(f"Error scanning {ip}: {e}")

    except ValueError as e:
        print(f"Error: Invalid subnet format. {e}")
    except KeyboardInterrupt:
        print("\nScan interrupted by user")

if __name__ == "__main__":
    main() 