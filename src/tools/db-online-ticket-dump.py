#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

import argparse
import re
import requests

parser = argparse.ArgumentParser(description='DB online ticket API response output')
parser.add_argument('--name', type=str, required=True, help='Case-sensitive last name, as on the ticket')
parser.add_argument('--ref', type=str, required=True, help='12 digit Auftragsnummer')
arguments = parser.parse_args()

postData = f'<rqfindorder version="1.0"><rqheader v="23080000" os="KCI" app="NAVIGATOR"/><rqorder on="{arguments.ref}"/><authname tln="{arguments.name}"/></rqfindorder>'
print(f"Request 1:\n{postData}\n")
req = requests.post('https://fahrkarten.bahn.de/mobile/dbc/xs.go?', data=postData)
print(f"Reply 1:\n{req.text}\n")

kwid = re.search(r'kwid="([^"]*)"', req.text).group(1)
postData = f'<rqorderdetails version="1.0"><rqheader v="23040000" os="KCI" app="KCI-Webservice"/><rqorder on="{arguments.ref}" kwid="{kwid}"/><authname tln="{arguments.name}"/></rqorderdetails>'
print(f"Request 2:\n{postData}\n")
req = requests.post('https://fahrkarten.bahn.de/mobile/dbc/xs.go?', data=postData)
print(f"Reply 2:\n{req.text}")
