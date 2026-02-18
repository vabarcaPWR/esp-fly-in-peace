#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
MICRO_DIR="${ROOT_DIR}/micro"
PORT="/dev/ttyUSB0"
PORT_EXPLICIT=false
FORCE_RELEASE_PORT=false

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

usage()
{
	echo "Usage: $0 [-p PORT] [-f|--force-release-port]"
	echo "  -p PORT    Serial port (default: /dev/ttyUSB0)"
	echo "  -f         Force release busy port using fuser -k"
	echo "  --force-release-port  Same as -f"
}

find_serial_port()
{
	for candidate in /dev/ttyUSB* /dev/ttyACM*; do
		if [[ -e "$candidate" ]]; then
			echo "$candidate"
			return 0
		fi
	done

	return 1
}

filtered_args=()
while [[ $# -gt 0 ]]; do
	case "$1" in
		--force-release-port)
			FORCE_RELEASE_PORT=true
			shift
			;;
		*)
			filtered_args+=("$1")
			shift
			;;
	esac
done
set -- "${filtered_args[@]}"

while getopts ":p:hf" opt; do
	case "$opt" in
		p)
			PORT="$OPTARG"
			PORT_EXPLICIT=true
			;;
		f)
			FORCE_RELEASE_PORT=true
			;;
		h)
			usage
			exit 0
			;;
		:)
			echo -e "${RED}[ERROR]${NC} Option -${OPTARG} requires an argument." >&2
			usage >&2
			exit 1
			;;
		\?)
			echo -e "${RED}[ERROR]${NC} Invalid option: -${OPTARG}" >&2
			usage >&2
			exit 1
			;;
	esac
done

if ! command -v idf.py >/dev/null 2>&1; then
	if [[ -f "${SCRIPT_DIR}/env.sh" ]]; then
		echo -e "${YELLOW}[ENV]${NC} idf.py not found. Sourcing scripts/env.sh..."
		# shellcheck source=/dev/null
		source "${SCRIPT_DIR}/env.sh"
	fi
fi

if ! command -v idf.py >/dev/null 2>&1; then
	echo -e "${RED}[ERROR]${NC} idf.py is not available. Source ESP-IDF env first:" >&2
	echo "  source ./scripts/env.sh" >&2
	exit 1
fi

if [[ ! -e "$PORT" ]]; then
	if [[ "$PORT_EXPLICIT" == true ]]; then
		echo -e "${RED}[ERROR]${NC} Selected port does not exist: ${PORT}" >&2
		exit 1
	fi

	if detected_port="$(find_serial_port)"; then
		echo -e "${YELLOW}[PORT]${NC} ${PORT} not found. Using detected port: ${detected_port}"
		PORT="$detected_port"
	else
		echo -e "${RED}[ERROR]${NC} No serial port detected (/dev/ttyUSB* or /dev/ttyACM*)." >&2
		echo "  Connect the board and retry, or pass one explicitly with -p." >&2
		exit 1
	fi
fi

if command -v fuser >/dev/null 2>&1 && fuser "$PORT" >/dev/null 2>&1; then
	if [[ "$FORCE_RELEASE_PORT" == true ]]; then
		echo -e "${YELLOW}[PORT]${NC} Releasing busy port: ${PORT}"
		fuser -k "$PORT" >/dev/null 2>&1 || true
		sleep 1
		if command -v fuser >/dev/null 2>&1 && fuser "$PORT" >/dev/null 2>&1; then
			echo -e "${RED}[ERROR]${NC} Unable to release busy port: ${PORT}" >&2
			echo "  Try closing the process manually and retry." >&2
			exit 1
		fi
	else
		echo -e "${RED}[ERROR]${NC} Port is busy: ${PORT}" >&2
		echo "  Process using the port:" >&2
		fuser "$PORT" >&2 || true
		echo "  Close the process manually or retry with --force-release-port." >&2
		exit 1
	fi
fi

cd "$MICRO_DIR"

echo -e "${GREEN}[PHASE3.7]${NC} Running quick firmware check on ${PORT}"
echo -e "${YELLOW}[MANUAL]${NC} In nRF Connect, execute:"
echo "  1) Connect to FlyInPeace"
echo "  2) Enable Notify on 6E400003-..."
echo "  3) Write PING to 6E400002-..."
echo "  4) Disconnect and reconnect once"
echo

echo -e "${GREEN}[FLASH+MONITOR]${NC} Starting idf.py -p ${PORT} flash monitor"
idf.py -p "$PORT" flash monitor
