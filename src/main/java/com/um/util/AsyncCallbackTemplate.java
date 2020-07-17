package com.um.util;

import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.MoreExecutors;

import javax.annotation.ParametersAreNonnullByDefault;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

/**
 * AsyncCallbackTemplate
 *
 * @author Eniso
 */
public class AsyncCallbackTemplate {

    public static <T> void withCallbackAndTimeout(
            ListenableFuture<T> future,
            Consumer<T> onSuccess, Consumer<Throwable> onFailure,
            long timeoutInMs, ScheduledExecutorService timeoutExecutor,
            Executor callbackExecutor) {
        future = Futures.withTimeout(future, timeoutInMs, TimeUnit.MILLISECONDS, timeoutExecutor);
        withCallback(future, onSuccess, onFailure, callbackExecutor);
    }

    public static <T> void withCallback(
            ListenableFuture<T> future,
            Consumer<T> onSuccess, Consumer<Throwable> onFailure) {
        withCallback(future, onSuccess, onFailure, null);
    }

    public static <T> void withCallback(
            ListenableFuture<T> future,
            Consumer<T> onSuccess, Consumer<Throwable> onFailure,
            Executor executor) {
        FutureCallback<T> callback = new FutureCallback<T>() {
            @Override
            public void onSuccess(T result) {
                try {
                    onSuccess.accept(result);
                } catch (Throwable th) {
                    onFailure(th);
                }
            }

            @Override
            @ParametersAreNonnullByDefault
            public void onFailure(Throwable t) {
                onFailure.accept(t);
            }
        };

        Futures.addCallback(future, callback, (executor != null) ? executor : MoreExecutors.directExecutor());
    }

}
