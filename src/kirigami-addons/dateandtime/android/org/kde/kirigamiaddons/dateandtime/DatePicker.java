/*
 *  SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

package org.kde.kirigamiaddons.dateandtime;

import android.app.DatePickerDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.app.DialogFragment;
import android.app.Activity;

import java.util.Calendar;

public class DatePicker extends DialogFragment implements DatePickerDialog.OnDateSetListener {

    private Activity activity;
    private long initialDate;

    private native void dateSelected(int day, int month, int year);
    private native void cancelled();

    public DatePicker(Activity activity, long initialDate) {
        super();
        this.activity = activity;
        this.initialDate = initialDate;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Calendar cal = Calendar.getInstance();
        cal.setTimeInMillis(initialDate);
        DatePickerDialog dialog = new DatePickerDialog(activity, this, cal.get(Calendar.YEAR), cal.get(Calendar.MONTH), cal.get(Calendar.DAY_OF_MONTH));
        android.widget.DatePicker picker = dialog.getDatePicker();
        return dialog;
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        cancelled();
    }

    @Override
    public void onDateSet(android.widget.DatePicker view, int year, int month, int day) {
        // Android reports month starting with 0
        month++;
        dateSelected(day, month, year);
    }

    public void doShow() {
        show(activity.getFragmentManager(), "datePicker");
    }
}
