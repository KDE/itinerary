#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
import time
from appium import webdriver
from appium.webdriver.common.appiumby import AppiumBy

import os

class PassTest(ItineraryTestCase):
    def setUp(self):
        # open the passes & programs page
        self.driver.find_element(by=AppiumBy.NAME, value="Passes & Programs").click()

    def test_programMembership(self):
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Import...").is_displayed())
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Import...").is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Import...").click()
        self.openFile("data/bahncard.json")
        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Import...").is_displayed())
        self.driver.find_element(by=AppiumBy.NAME, value="BahnCard 25 (2. Kl.) (BC25)").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Edit").click()

        saveButton = self.driver.find_element(by=AppiumBy.NAME, value="Save")
        self.assertTrue(saveButton.is_enabled())

        nameInput = self.driver.find_element(by=AppiumBy.NAME, value="Name")
        nameInput.clear()
        self.assertFalse(saveButton.is_enabled())
        nameInput.send_keys("BahnCard 50")
        self.assertTrue(saveButton.is_enabled())
        saveButton.click();

        self.goBack()
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="BahnCard 50").is_displayed())

        self.driver.find_element(by=AppiumBy.NAME, value="BahnCard 50").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Delete").click()

        deleteDialog = self.driver.find_element(by=AppiumBy.NAME, value="Delete Pass")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.NAME, value="Delete").click()
        # TODO how to check this element does not exist anymore?
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="BahnCard 25 (2. Kl.) (BC25)").is_displayed())

    def test_addProgramMembership(self):
        self.openContextDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Add Program Membership...").click()
        saveButton = self.driver.find_element(by=AppiumBy.NAME, value="Save")
        self.assertFalse(saveButton.is_enabled())

        self.driver.find_element(by=AppiumBy.NAME, value="Name").send_keys("New Discount Card")
        self.assertTrue(saveButton.is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Member").send_keys("Konqi Dragon")
        self.driver.find_element(by=AppiumBy.NAME, value="Number").send_keys("1234567890")
        # TODO date/time input fields not accessible yet
        self.assertTrue(saveButton.is_enabled())
        saveButton.click()

        self.driver.find_element(by=AppiumBy.NAME, value="New Discount Card").click()
        page = self.driver.find_element(by=AppiumBy.NAME, value="Program Membership")
        self.assertTrue(page.find_element(by=AppiumBy.NAME, value="New Discount Card").is_displayed())
        self.assertTrue(page.find_element(by=AppiumBy.NAME, value="1234567890").is_displayed())
        self.assertTrue(page.find_element(by=AppiumBy.NAME, value="Konqi Dragon").is_displayed())

        self.driver.find_element(by=AppiumBy.NAME, value="Delete").click()
        deleteDialog = self.driver.find_element(by=AppiumBy.NAME, value="Delete Pass")
        self.assertTrue(deleteDialog.is_displayed())
        deleteDialog.find_element(by=AppiumBy.NAME, value="Delete").click()

if __name__ == '__main__':
    unittest.main()
