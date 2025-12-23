from django.contrib import messages
from django.contrib.auth import logout as auth_logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.forms import UserCreationForm
from django.shortcuts import redirect, render
from django.views.decorators.http import require_GET, require_POST


def register(request):
    if request.method == "POST":
        form = UserCreationForm(request.POST)
        if form.is_valid():
            user = form.save()
            username = form.cleaned_data.get("username")
            messages.success(
                request, f"Аккаунт {username} успешно создан! Теперь вы можете войти."
            )
            return redirect("login")
        else:
            for field, errors in form.errors.items():
                for error in errors:
                    messages.error(request, f"{field}: {error}")
    else:
        form = UserCreationForm()

    return render(request, "accounts/register.html", {"form": form})


def custom_logout(request):
    auth_logout(request)
    messages.success(request, "Вы успешно вышли из системы.")
    return redirect("home")


@login_required
def profile(request):
    user_articles = request.user.article_set.all()
    return render(request, "accounts/profile.html", {"user_articles": user_articles})
