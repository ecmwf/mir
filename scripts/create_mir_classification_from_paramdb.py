#!/usr/bin/env python3
"""
Script to create parameter-class.yaml for MIR from parameter database.

Database credentials should be set as environment variables:
    PARAM_DB_HOST    - IP address of the MySQL database server
    PARAM_DB_USER_RO - User name (read-only access)
    PARAM_DB_PASS_RO - Password for the database user

If not set, the script will prompt for them interactively.

robert.osinski@ecmwf.int 2023-10-17
"""

import argparse
import getpass
from os import environ
from sys import exit

import mysql.connector
import yaml


def get_credentials():
    """Get database credentials from environment or prompt user."""
    host = environ.get("PARAM_DB_HOST")
    user = environ.get("PARAM_DB_USER_RO")
    password = environ.get("PARAM_DB_PASS_RO")

    if not host:
        host = input("Enter database host (e.g. webapps-db-prod): ").strip()
        if not host:
            exit("Error: database host is required")
    if not user:
        user = input("Enter database user: ").strip()
        if not user:
            exit("Error: database user is required")
    if not password:
        password = getpass.getpass("Enter database password: ")

    return host, user, password


def create_mir_classification_from_paramdb(host, user, password, database="param") -> dict:
    """Create MIR classification from the parameter database."""
    conn = mysql.connector.connect(
        host=host, user=user, password=password, database=database
    )
    cur = conn.cursor()

    # Sort out parameters marked as retired in table param
    query = """
        SELECT mir_classification.param_id, mir_classifiers.classifier_name
        FROM mir_classification
        INNER JOIN mir_classifiers
            ON mir_classification.mir_classifier = mir_classifiers.classifier
        JOIN param
            ON param.id = mir_classification.param_id
        WHERE param.retired = 0
    """
    cur.execute(query)
    results = cur.fetchall()
    conn.close()

    # Build classification dictionary
    paramids = [row[0] for row in results]
    classifiers = [row[1] for row in results]

    classifiers_uniq = sorted(set(classifiers))

    output_dict = {}
    for classifier in classifiers_uniq:
        indices = [i for i, c in enumerate(classifiers) if c == classifier]
        pids = [paramids[i] for i in indices]
        print(f"{classifier}: {pids}")
        output_dict[classifier] = pids

    return output_dict


def write_yaml(output_dict, filename):
    """Write classification dictionary to YAML file."""
    with open(filename, "w") as f:
        f.write("---\n")
        f.write("# yamllint disable rule:line-length\n")
        yaml.dump(output_dict, f, default_flow_style=False, sort_keys=True)


def parameter_names(host, user, password, database="param"):
    """Fetch parameter id to name mapping from the database."""
    conn = mysql.connector.connect(
        host=host, user=user, password=password, database=database
    )
    cur = conn.cursor()
    cur.execute("SELECT id, name FROM param ORDER BY id")
    results = {row[0]: row[1] for row in cur.fetchall()}
    conn.close()
    return results


def parameter_comments(filename, param_names):
    """Add parameter names as comments after paramId values."""
    with open(filename, "r") as f:
        lines = f.readlines()

    with open(filename, "w") as f:
        for line in lines:
            # Match lines like "- 12345" (list items with param IDs)
            if line.startswith("- "):
                try:
                    param_id = int(line.strip().split()[1])
                    name = param_names.get(param_id, "")
                    if name:
                        line = line.rstrip() + f"  # {name}\n"
                except (ValueError, IndexError):
                    pass
            f.write(line)


def main():
    parser = argparse.ArgumentParser(
        description="Create parameter-class.yaml for MIR from parameter database.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Database credentials:
  Set these environment variables (recommended for security):
    PARAM_DB_HOST    - Database host
    PARAM_DB_USER_RO - Database user (read-only)
    PARAM_DB_PASS_RO - Database password

  If not set, the script will prompt interactively.
""",
    )
    parser.add_argument(
        "-o", "--output",
        default="parameter-class.yaml",
        help="Output YAML filename (default: parameter-class.yaml)",
    )
    parser.add_argument(
        "-d", "--database",
        default="param",
        help="Database name (default: param)",
    )
    parser.add_argument(
        "--no-comments",
        action="store_true",
        help="Skip adding parameter name comments to output",
    )

    args = parser.parse_args()

    host, user, password = get_credentials()

    print(f"Connecting to {host}...")
    output_dict = create_mir_classification_from_paramdb(host, user, password, args.database)

    print(f"Write to '{args.output}'...")
    write_yaml(output_dict, args.output)

    if not args.no_comments:
        print("Adding parameter comments...")
        names = parameter_names(host, user, password, args.database)
        parameter_comments(args.output, names)
    print("Done.")


if __name__ == "__main__":
    main()
